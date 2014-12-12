#include <stdint.h>
#include "../preemptive-scheduler/phyAlloc.h"
	
#define PAGE_SIZE			 (4*1024*8)// 4KB
#define SECON_LVL_TT_COUN 	 256 // 2⁸=256 entrees au lvl 2
#define SECON_LVL_TT_SIZE 	 ((SECON_LVL_TT_COUN * 32)/8) // taille du lvl 2 en octets = 1024 octets
#define FIRST_LVL_TT_COUN 	 4096 // 2^12=4096 entrees au lvl 1 
#define FIRST_LVL_TT_SIZE 	 ((FIRST_LVL_TT_COUN * 32)/8) // taille du lvl 1 en octets = 16384 octets
#define TOTAL_TT_SIZE 		 (SECON_LVL_TT_SIZE * FIRST_LVL_TT_COUN + FIRST_LVL_TT_SIZE)


uint32_t first_level_flags= 1; // 10 bits de flags, seul le dernier est à 1 (toujours)

uint32_t device_flags= 
	(0 << 12) | //nG 1 si entrée valide pour tous les processus
	(0 << 11) | //S 1 si entrée partagée
	(0 << 10) | //APX (dépend des permissions)
	(0b000 << 6) | //TEX (cf 8.4.2 du sujet)
	(0b11 << 4) | //AP (dépend des permissions)
	(0b00 << 2)| // CB (dépend de TEX)
	(1<<1) | // Toujours 1
	0 ; // XN 1 si contient du code exécutable
	
unsigned int pointer_pagetable_lvl1=0x48000;
unsigned int pointer_start_lvl2=0x52001; // adresse de départ de la table de niveau 2
// Level 1 : 0x48000 -> 0x52000
// Level 2 : 0x52001 -> 0x452001

void start_mmu_C()
{
	register unsigned int control;
	__asm("mcr p15, 0, %[zero], c1, c0, 0" : : [zero] "r"(0)); //Disable cache
	__asm("mcr p15, 0, r0, c7, c7, 0"); //Invalidate cache (data and instructions) */
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
	//total++;
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
	
unsigned int init_kern_translation_table (void) {	
	
	// Espace logique 0xC0000...
	// TODO flags
	
	int i=0;
	int *adrLvl2=(int *)pointer_start_lvl2; 
	
	// Remplissage de la table de niveau 1
	for(i=pointer_pagetable_lvl1; i<pointer_pagetable_lvl1+FIRST_LVL_TT_SIZE; i+=4)
	{
		int** tmp=(int **)i;
		(*tmp) =  (int*)(((int)adrLvl2 << 10) | first_level_flags);
		adrLvl2 += SECON_LVL_TT_COUN;
	}

			
	 // Adresses physiques entre 0x0 et 0x500000 -> p entre 0x0 et 0x500
	for(i=0; i<0x500; i++)	{	
		int indexEntreeLvl1=i/256;
		int offsetLvl2 = i%256;
		int** pointeurCaseLvl1= (int **)(pointer_pagetable_lvl1) + indexEntreeLvl1;
		int* pointeurTableLvl2=(*pointeurCaseLvl1);
		int* pointeurEntreeLvl2 = pointeurTableLvl2 + offsetLvl2;
		
		(*pointeurEntreeLvl2)=(i<< 13) | device_flags;
	}
	// Adresses physiques entre 0x20000000 et 0x20FFFFFF -> p entre 0x20000 et 0x20FFF
		for(i=0x20000; i<0x20fff; i++)	{
		int indexEntreeLvl1=i/256;
		int offsetLvl2 = i%256;
		int** pointeurCaseLvl1= (int **)(pointer_pagetable_lvl1) + indexEntreeLvl1;
		int* pointeurTableLvl2=(*pointeurCaseLvl1);
		int* pointeurEntreeLvl2 = pointeurTableLvl2 + offsetLvl2;
		
		(*pointeurEntreeLvl2)=(i << 13) | device_flags;
	}
	
	start_mmu_C();
	configure_mmu_C();
			
	return 0;
}
