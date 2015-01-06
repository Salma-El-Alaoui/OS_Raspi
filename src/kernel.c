#include "preemptive-scheduler/hw.h"
#include "preemptive-scheduler/sched.h"
#include <stdlib.h>
#include "mmu/translate.c"

unsigned int init_kern_translation_table (void);

void funcA()
{
	int cptA = 0;
	while (1) {
		cptA ++;
	}

}
void funcB()
{
	int cptB = 1;
	while (cptB<10) {
		cptB += 2 ;
	}
}
//------------------------------------------------------------------------
int kmain ( void )
{
	unsigned int test_pa;
	init_hw();
	create_process(funcB, NULL, STACK_SIZE);
	create_process(funcA,NULL, STACK_SIZE);
	start_sched();
	init_kern_translation_table ();
	configure_mmu_C();
	test_pa=translate(0x8728);
	start_mmu_C();
	while(1);
	/* Pas atteignable vues nos 2 fonctions */
	return 0;
}
