#include <stdint.h>
	
#define PAGE_SIZE			 (4*1024*8)// 4KB
#define SECON_LVL_TT_COUN 	 256 // 2⁸=256 entrees au lvl 2
#define SECON_LVL_TT_SIZE 	 ((SECON_LVL_TT_COUN * 32)/8) // taille du lvl 2 en octets = 1024 octets
#define FIRST_LVL_TT_COUN 	 4096 // 2^12=4096 entrees au lvl 1 
#define FIRST_LVL_TT_SIZE 	 ((FIRST_LVL_TT_COUN * 32)/8) // taille du lvl 1 en octets = 16384 octets
#define TOTAL_TT_SIZE 		 (SECON_LVL_TT_SIZE * FIRST_LVL_TT_COUN + FIRST_LVL_TT_SIZE)

static int total= TOTAL_TT_SIZE;

uint32_t first_level_flags= 1; // 10 bits de flags, seul le dernier est à 1 (toujours)

uint32_t normal_flags= 
	(0 << 12) | //nG 1 si entrée valide pour tous les processus
	(0 << 11) | //S 1 si entrée partagée
	(0 << 10) | //APX (dépend des permissions)
	(0b001 << 6) | //TEX (cf 8.4.2 du sujet)
	(0b11 << 4) | //AP (dépend des permissions)
	(0b00 << 2)| // CB (dépend de TEX)
	(1<<1) | // Toujours 1
	0 ; // XN 1 si contient du code exécutable
	
uint32_t device_flags= 
	(0 << 12) | //nG 1 si entrée valide pour tous les processus
	(0 << 11) | //S 1 si entrée partagée
	(0 << 10) | //APX (dépend des permissions)
	(0b000 << 6) | //TEX (cf 8.4.2 du sujet)
	(0b11 << 4) | //AP (dépend des permissions)
	(0b01 << 2)| // CB (dépend de TEX)
	(1<<1) | // Toujours 1
	0 ; // XN 1 si contient du code exécutable

// Level 1 : 0x48000 -> 0x52000
// Level 2 : 0x52000 -> 0x452000	
unsigned int pointer_pagetable_lvl1=0x48000;
unsigned int pointer_start_lvl2=0x52000; // adresse de départ de la table de niveau 2

// Table occupation des frame 0x452000 -> 0x473000
unsigned int pointer_table_occup_frame=0x452000;
unsigned int end_table_occup_frame=0x473000;

void start_mmu_C()
{
	
	register unsigned int control;
	__asm("mcr p15, 0, %[zero], c1, c0, 0" : : [zero] "r"(0)); //Disable cache
	__asm("mcr p15, 0, r0, c7, c7, 0"); //Invalidate cache (data and instructions) 
	__asm("mcr p15, 0, r0, c8, c7, 0"); //Invalidate TLB entries

	/* Enable ARMv6 MMU features (disable sub-page AP) */
	control = (1<<23) | (1 << 15) | (1 << 4) | 1;

	/* Invalidate the translation lookaside buffer (TLB) */
	__asm volatile("mcr p15, 0, %[data], c8, c7, 0" : : [data] "r" (0));

	/* Write control register */
	__asm volatile("mcr p15, 0, %[control], c1, c0, 0" : : [control] "r" (control));
}

void configure_mmu_C()
{
	register unsigned int pt_addr = pointer_pagetable_lvl1;
	total++;
	/* Translation table 0 */
	__asm volatile("mcr p15, 0, %[addr], c2, c0, 0" : : [addr] "r" (pt_addr));

	/* Translation table 1 */
	__asm volatile("mcr p15, 0, %[addr], c2, c0, 1" : : [addr] "r" (pt_addr));

	/* Use translation table 0 for everything */
	__asm volatile("mcr p15, 0, %[n], c2, c0, 2" : : [n] "r" (0));

	/* Set Domain 0 ACL to "Manager", not enforcing memory permissions
	* Every mapped section/page is in domain 0
	*/
	__asm volatile("mcr p15, 0, %[r], c3, c0, 0" : : [r] "r" (0x3));
}

void init_table_occup_frame()
{
	uint8_t* i;
	for(i=(uint8_t*)pointer_table_occup_frame; i<(uint8_t*)end_table_occup_frame; i++){
		if(i<(pointer_table_occup_frame + (0x473000/0x1000)))
		{
			// Espace déjà utilisé (stack, interrupt vectors, page table...)
			(*i)=1;
		}
		else
		{
			(*i) = 0; // default : frame is free
		}	
	}
}


uint8_t* vMem_Alloc(unsigned int nbPages){
	
	int space = 0;
	uint8_t* start_space_in_table = 0;
	int numDernierePage= 0;
	uint8_t* i = (uint8_t*)pointer_table_occup_frame;
	while(i<(uint8_t*)end_table_occup_frame && space < nbPages){
		if((*i) == 0){
			if (space == 0){
				start_space_in_table = i;
			}
			space++;
			
		} else {
			 space = 0;
		}
		i++;
		numDernierePage++;
	}
	if (space == nbPages) {
		int j;
		for(j=0; j<nbPages; j++)
		{
			uint8_t* frame = (uint8_t*)((int)start_space_in_table+j);
			(*frame)=1;
		}
		return (uint8_t*)((numDernierePage - nbPages) << 12) ; // adresse logique première page allouée = numéro de la première page + 12 bits à 0 (index dans la page)
	}
	return (uint8_t*)(-1); // Erreur
}

void vMem_Free(uint8_t* ptr, unsigned int nbPages){
	int numPage= (int)ptr >>12;
	int i;
	for(i=numPage; i<numPage+nbPages; i++)
	{
		uint8_t* frame_to_free= (uint8_t*) ((int)pointer_table_occup_frame+i);
		*(frame_to_free)=0;
	}
}
	
unsigned int init_kern_translation_table (void) {	
	
	int i=0;
	int *adrLvl2=(int *)(pointer_start_lvl2); 
	
	// Remplissage de la table de niveau 1
	for(i=pointer_pagetable_lvl1; i<pointer_pagetable_lvl1+FIRST_LVL_TT_SIZE; i+=4)
	{
		int** tmp=(int **)i;
		// On enlève les 10 derniers bits pour obtenir la coarse page table base address
		int baseAddrLvl2 = ((int)adrLvl2 >> 10); 
		(*tmp) =  (int*)((baseAddrLvl2 << 10) | first_level_flags);
		adrLvl2 += SECON_LVL_TT_COUN;
	}

			
	 // Adresses physiques entre 0x0 et 0x500000 -> p entre 0x0 et 0x500
	for(i=0; i<0x500; i++)	{
		int indexEntreeLvl1=i/256;
		int offsetLvl2 = i%256;
		int* pointeurCaseLvl1= (int *)(pointer_pagetable_lvl1) + indexEntreeLvl1;
		// On efface les 10 bits de flags
		int* pointeurTableLvl2=(int*)((*pointeurCaseLvl1)& 0xFFFFFC00);
		// On ajoute le second-level table index et les 2 bits à 0 pour obtenir le second-level descriptor address
		int* pointeurEntreeLvl2 =(int*)( (int)pointeurTableLvl2 | (offsetLvl2 << 2));
		//On remplit le second level descriptor
		(*pointeurEntreeLvl2)=(i<< 12) | normal_flags;
	}
	// Adresses physiques entre 0x20000000 et 0x20FFFFFF -> p entre 0x20000 et 0x20FFF
		for(i=0x20000; i<0x20fff; i++)	{
		int indexEntreeLvl1=i/256;
		int offsetLvl2 = i%256;
		int* pointeurCaseLvl1= (int *)(pointer_pagetable_lvl1) + indexEntreeLvl1;
		// On efface les 10 bits de flags
		int* pointeurTableLvl2=(int*)((*pointeurCaseLvl1)& 0xFFFFFC00);
		// On ajoute le second-level table index et les 2 bits à 0 pour obtenir le second-level descriptor address
		int* pointeurEntreeLvl2 =(int*)( (int)pointeurTableLvl2 | (offsetLvl2 << 2));
		//On remplit le second level descriptor
		(*pointeurEntreeLvl2)=(i << 12) | device_flags;
	}
	

	configure_mmu_C();
	//test_pa=translate(0x8728);
	start_mmu_C();
	init_table_occup_frame();
				
	return 0;
}
