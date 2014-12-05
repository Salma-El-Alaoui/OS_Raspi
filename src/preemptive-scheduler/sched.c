#include "sched.h"
#include "phyAlloc.h"
#include "hw.h"
#include <stdlib.h>

struct pcb_s * current_pcb = NULL;
pcb_s * firstTime_pcb;

void init_pcb(struct pcb_s * pcb,func_t f, void* args, unsigned int stack_size)
{
	pcb->instruct_address = (unsigned int) &start_current_process;
	pcb->stack_base = (unsigned int) phyAlloc_alloc(stack_size);
	pcb->stack_pointer = pcb->stack_base + stack_size - sizeof(int);
	
	// On stocke CPRS, 13 veut dire mode system
	(*(unsigned int*)pcb->stack_pointer) = 0x53;
	// On stocke LR
	pcb->stack_pointer += -sizeof(int);
	(*(unsigned int*)pcb->stack_pointer) = &start_current_process;
	// On dépile 13 registres au premier switch réel, on se décale/remonte dans la pile de 14 cases	
	pcb->stack_pointer += -14*sizeof(int);
	pcb->stack_size = stack_size;
	
	pcb->f=f;
	pcb->args = args;
	
	pcb->etatP = READY;
}

void create_process(func_t f, void* args, unsigned int stack_size)
{
	pcb_s * pcb = phyAlloc_alloc(sizeof(pcb_s));
	
	if(current_pcb == NULL)
	{
		current_pcb = pcb;
		pcb->pcbNext = pcb;
	}
	else
	{
		pcb->pcbNext = current_pcb->pcbNext;
		current_pcb->pcbNext = pcb;
	}
	
	init_pcb(pcb,f,args,stack_size);
}

void start_current_process()
{
	current_pcb->etatP= RUNNING;
	current_pcb->f(current_pcb->args);
	current_pcb->etatP = TERMINATED;
	while(1);
    //ctx_switch();
}

void elect()
{
	while(current_pcb->pcbNext->etatP == TERMINATED)
	{
		/*if(current_pcb->pcbNext == current_pcb)//Cas limite, le process terminé boucle sur lui-même
		{
			current_pcb = firstTime_pcb;
			
		}*/
		pcb_s *old_pcb = current_pcb->pcbNext;	
		current_pcb->pcbNext = old_pcb->pcbNext;
	 
		phyAlloc_free((void *)old_pcb->stack_base, old_pcb->stack_size);
		phyAlloc_free(old_pcb, sizeof(pcb_s));
	}
	current_pcb=current_pcb->pcbNext;
}

void start_sched()
{
	// firstTime_pcb = phyAlloc_alloc(sizeof(pcb_s));
	firstTime_pcb->pcbNext = current_pcb;
	current_pcb = firstTime_pcb;
	
	//On arme le timer
	set_tick_and_enable_timer();
	//On dit que la suite du code est interruptible
	ENABLE_IRQ();
}
	
void __attribute__ ((naked)) ctx_switch()
{
	DISABLE_IRQ();
	//Save the context
	__asm("srsdb sp!, #0x13");
	__asm("push {r0-r12}");
	__asm("mov %0, sp" : "=r"(current_pcb->stack_pointer));

	//elect a new process
	elect();
	//restore the context of the elected process
	__asm("mov sp, %0" : : "r"(current_pcb->stack_pointer));
	__asm("pop {r0-r12}");
	set_tick_and_enable_timer();
	ENABLE_IRQ();
	__asm("rfeia sp!");
}

void ctx_switch_from_irq()
{
	//Fait pointer lr vers l'instruction interrompue
	//On fait -4 car si on est interrompu en plein milieu d'une instruction, il faut la refaire
	__asm("sub lr, lr, #4");
	
	//Sauvegarde lr et cpsr dans la pile du mode system
	//C'est une astuce pour sauvegarder des trucs dans le system alors qu'on est en irq
	__asm("srsdb sp!, #0x13");
	
	//Repasse en mode system
	__asm("cps #0x13");	

	//sauvegarde 
	__asm("push {r0-r12}");
	
	//Différent du lr au dessus car on a changé de mode
	//__asm("mov %0, lr" : "=r"(current_pcb->instruct_address));
	__asm("mov %0, sp" : "=r"(current_pcb->stack_pointer));

	//choix nouveau processus
	elect();
	
	__asm("mov sp, %0" : : "r"(current_pcb->stack_pointer));
	__asm("pop {r0-r12}");
	
	//On arme le timer
	set_tick_and_enable_timer();
	//On dit que la suite du code est interruptible
	ENABLE_IRQ();

	// Jump -> On met la valeur de lr dans PC
	__asm("rfeia sp!");

	
}
