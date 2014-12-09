#include "syscall.h"
#include "../preemptive-scheduler/sched.h"
#include "../preemptive-scheduler/hw.h"

unsigned int numSysCall;

void doSysCallReboot(){
	const int PM_RSTC = 0x2010001c;
	const int PM_WDOG = 0x20100024;
	const int PM_PASSWORD = 0x5a000000;
	const int PM_RSTC_WRCFG_FULL_RESET = 0x00000020;
	PUT32(PM_WDOG, PM_PASSWORD | 1);
	PUT32(PM_RSTC, PM_PASSWORD | PM_RSTC_WRCFG_FULL_RESET);
	ENABLE_IRQ();
	while (1);
}

void doSysCallWait(){
	unsigned int nbQuantums;
	__asm("mov %0, r1" : "=r"(nbQuantums));
	wait(nbQuantums);
//TODO : Rendre le process inéligible pour une durée nbQuantums.
	
}


void SWIHandler()
{
    unsigned int sysCallInR0;
    __asm("mov %0, r0" : "=r"(sysCallInR0));
  
	switch(sysCallInR0)
	{
		case 1 : 
			doSysCallReboot();
			break;
		case 2 : 
			doSysCallWait();
			break;
	}
		
	

}

void sys_reboot()
{
	DISABLE_IRQ();
	numSysCall = 1;
	__asm("mov r0, %0" : : "r"(numSysCall) : "r0");
	__asm("SWI 0" : : : "lr");

}


void sys_wait(unsigned int nbQuantums)
{
	DISABLE_IRQ();
	numSysCall = 2;
	__asm("mov r0, %0" : : "r"(numSysCall) : "r0");
	__asm("mov r1, %0" : : "r"(nbQuantums) : "r1");
	__asm("SWI 0" : : : "lr");
	ENABLE_IRQ();
}


