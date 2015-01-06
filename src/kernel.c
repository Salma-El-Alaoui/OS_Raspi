#include "preemptive-scheduler/hw.h"
#include "preemptive-scheduler/sched.h"
#include <stdlib.h>
#include <stdint.h>

unsigned int init_kern_translation_table (void);
uint8_t* vMem_Alloc(unsigned int nbPages);
void vMem_Free(uint8_t* ptr, unsigned int nbPages);

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
	init_hw();
	init_kern_translation_table ();
	create_process(funcB, NULL, STACK_SIZE);
	create_process(funcA,NULL, STACK_SIZE);
	start_sched();

	while(1);
	/* Pas atteignable vues nos 2 fonctions */
	return 0;
}
