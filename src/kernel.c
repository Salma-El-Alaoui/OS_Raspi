#include "scheduler/sched.h"
#include "scheduler/hw.h"
#include "syscalls/syscall.h"
#include <stdlib.h>

void funcA()
{
	int cptA = 0;
	while (cptA<10000000) {
		cptA ++;		
	}

}

void funcB()
{
	int cptB = 1;
	sys_wait_pid(2);
	while (1) {
		cptB += 2 ;
	}
}

//------------------------------------------------------------------------
int kmain ( void )
{
	init_hw();

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
