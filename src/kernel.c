#include "preemptive-scheduler/hw.h"
#include "preemptive-scheduler/sched.h"
#include "syscalls/syscall.h"
#include <stdlib.h>
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
		__asm("mov r0, %0" : : "r"(56) : "r0");
		sys_wait(3);
		__asm("mov %0, r0" : "=r"(cptB));
	}
}
//------------------------------------------------------------------------
int kmain ( void )
{
	init_hw();
	create_process(funcB, NULL, STACK_SIZE);
	create_process(funcA,NULL, STACK_SIZE);
	start_sched();
	while(1);
	/* Pas atteignable vues nos 2 fonctions */
	return 0;
}
