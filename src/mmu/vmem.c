#include <stdint.h>
#include "../preemptive-scheduler/phyAlloc.h"
	
#define PAGE_SIZE			 (4*1024*8)// 4KB
#define SECON_LVL_TT_COUN 	 256 // 2⁸=256 entrees au lvl 2
#define SECON_LVL_TT_SIZE 	 ((SECON_LVL_TT_COUN * 32)/8) // taille du lvl 2 en octets
#define FIRST_LVL_TT_COUN 	 4096 // 2^12=4096 entrees au lvl 1
#define FIRST_LVL_TT_SIZE 	 ((FIRST_LVL_TT_COUN * 32)/8) // taille du lvl 1 en octets
#define TOTAL_TT_SIZE 		 (SECON_LVL_TT_SIZE * FIRST_LVL_TT_COUN)


uint32_t device_flags= 
	(0 << 12) | //nG 1 si entrée valide pour tous les processus
	(0 << 11) | //S 1 si entrée partagée
	(0 << 10) | //APX (dépend des permissions)
	(0b000 << 6) | //TEX (cf 8.4.2 du sujet)
	(0b11 << 4) | //AP (dépend des permissions)
	(0b00 << 2)| // CB (dépend de TEX)
	(1<<1) | // Toujours 1
	0 ; // XN 1 si contient du code exécutable
	
unsigned int pointer_pagetable_lvl1;

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
	// Allocation table de niveau 1
	pointer_pagetable_lvl1=(unsigned int) phyAlloc_alloc(FIRST_LVL_TT_SIZE);
	
	int i=0;
	
	// Allocation des tables de niveau 2
	for(i=pointer_pagetable_lvl1; i<pointer_pagetable_lvl1+FIRST_LVL_TT_SIZE; i++)
	{
		int* tmp=i;
		(*tmp)=(unsigned int) phyAlloc_alloc(SECON_LVL_TT_SIZE);
	}

			
	 // Adresses physiques entre 0x0 et 0x500000 -> p entre 0x0 et 0xA0001
	for(i=0; i<0xA0001; i++)	{
		int indexEntreeLvl1=i/256;
		int* pointeurEntreeLvl1 = pointer_pagetable_lvl1 + indexEntreeLvl1;
		int* entreeLvl2=(*pointeurEntreeLvl1);
		
		(*entreeLvl2)=i;
	}
	// Adresses physiques entre 0x20000000 et 0x20FFFFFF -> p entre 0x80000 et 0x83fff
		for(i=0x80000; i<0x83fff; i++)	{
		int indexEntreeLvl1=i/256;
		int* pointeurEntreeLvl1 = pointer_pagetable_lvl1 + indexEntreeLvl1;
		int* entreeLvl2=(*pointeurEntreeLvl1);
		
		(*entreeLvl2)=i;
	}
	
	start_mmu_C();
	configure_mmu_C();
			
	return 0;
}
