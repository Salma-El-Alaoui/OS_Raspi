#include "scheduler/sched.h"
#include "scheduler/hw.h"
#include "syscalls/syscall.h"
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
	while (1) {
		cptB += 2 ;
	}
}

//------------------------------------------------------------------------
int kmain ( void )
{
	init_hw();
	init_kern_translation_table ();

#ifdef PRIORITY_SCHED
	create_process_priority(funcB, NULL, STACK_SIZE, 5);
	create_process_priority(funcA, NULL, STACK_SIZE, 2);
#else
	create_process(funcB, NULL, STACK_SIZE);
	create_process(funcA, NULL, STACK_SIZE);
#endif
	start_sched();

	while(1);
	/* Pas atteignable vues nos 2 fonctions */
	return 0;
}
