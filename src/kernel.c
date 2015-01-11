#include "scheduler/sched.h"
#include "scheduler/hw.h"
#include "syscalls/syscall.h"
#include "video/fb.h"
#include "video/pwm.h"
#include <stdlib.h>
#include <stdint.h>



void funcA()
{
	int cptA = 0;
	while (1) {
		cptA++;
		led_on();
//		ctx_switch();		
	}

}

void funcB()
{
	int cptB = 1;
	while (1) {
		cptB++;
		led_off();
//		ctx_switch();		
	}
}

void funcC()
{
	int cptB = 1;
	while (1) {
		audio_test();
	}
}



//------------------------------------------------------------------------
int kmain ( void )
{
	init_hw();
	//FramebufferInitialize();



#ifdef PRIORITY_SCHED
	create_process_priority(funcB, NULL, STACK_SIZE, 2);
	create_process_priority(funcA, NULL, STACK_SIZE, 2);
	create_process_priority(funcC, NULL, STACK_SIZE, 2);
#else
	create_process(funcC, NULL, STACK_SIZE);
	create_process(funcB, NULL, STACK_SIZE);
	create_process(funcA, NULL, STACK_SIZE);
#endif
	start_sched();




	/* Pas atteignable vues nos 2 fonctions */


	while(1) {}


	return 0;
}
