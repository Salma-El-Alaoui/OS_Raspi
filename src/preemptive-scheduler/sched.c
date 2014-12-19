#include "sched.h"
#include "phyAlloc.h"
#include "hw.h"
#include <stdlib.h>

struct pcb_s * current_pcb = NULL;
pcb_s* priority_lists[PRIORITY_NUMBER];

//------------------------------------------PARTIE UTILS------------------------------------------------------------------

pcb_s* findProcessById(unsigned int process_id)
{
	struct pcb_s * pcb_temp;
	pcb_temp = current_pcb;
	do {		
		if(pcb_temp->ID == process_id)
		{
			return pcb_temp;
		}
		pcb_temp = pcb_temp->pcbNext;
	}
	while(pcb_temp != current_pcb);

	return NULL;
}

void restartAllWaitingPIDProcess(unsigned int process_id)
{
	struct pcb_s * pcb_temp;
	pcb_temp = current_pcb;
	do {		
		if(pcb_temp->pid_waiting == process_id)
		{
			pcb_temp->etatP = READY;
			pcb_temp->pid_waiting = -1;
		}
		pcb_temp = pcb_temp->pcbNext;
	}
	while(pcb_temp != current_pcb);

}




//------------------------------------------PARTIE INITIALISATION---------------------------------------------------------
void start_sched()
{

	//On arme le timer
	set_tick_and_enable_timer();
	//On dit que la suite du code est interruptible
	ENABLE_IRQ();
}

void start_current_process()
{
	current_pcb->etatP= RUNNING;
	current_pcb->f(current_pcb->args);
	current_pcb->etatP = TERMINATED;
	while(1);
}

void init_pcb(struct pcb_s * pcb,func_t f, void* args, unsigned int stack_size, unsigned short priority)

{
	static unsigned int process_id = 1;
	pcb->ID = process_id++;

	pcb->pid_waiting = -1;

	pcb->instruct_address = (unsigned int) &start_current_process;
	pcb->stack_base = (unsigned int) phyAlloc_alloc(stack_size);
	pcb->stack_pointer = pcb->stack_base + stack_size - sizeof(int);
	
	// On stocke CPRS, 13 veut dire mode system
	(*(unsigned int*)pcb->stack_pointer) = 0x53;
	// On stocke LR
	pcb->stack_pointer += -sizeof(int);
	(*(unsigned int*)pcb->stack_pointer) = &start_current_process;
	// On dépile 13 registres au premier switch réel, on se décale/remonte dans la pile de 14 cases	
	pcb->stack_pointer += -13*sizeof(int);
	pcb->stack_size = stack_size;
	
	pcb->f=f;
	pcb->args = args;
	
	pcb->etatP = READY;
	
	if(priority>=PRIORITY_NUMBER){
		pcb->priority= PRIORITY_NUMBER-1;
	} else {
		pcb->priority= priority;
	}

}

void create_process(func_t f, void* args, unsigned int stack_size, unsigned short priority)
{

	pcb_s * pcb = phyAlloc_alloc(sizeof(pcb_s));
	int i;
	
	init_pcb(pcb,f,args,stack_size, priority);
	if(priority_lists == NULL)
	{
		for(i=0;i<PRIORITY_NUMBER;i++){
			priority_lists[i] = NULL;
		}
	}
	if(priority_lists[pcb->priority] == NULL){
		priority_lists[pcb->priority] = pcb;
		pcb->pcbNext = pcb;
		pcb->pcbPrevious = pcb;
	}
	else
	{
		pcb->pcbNext = priority_lists[pcb->priority];
		pcb->pcbPrevious = priority_lists[pcb->priority]->pcbPrevious;
		priority_lists[pcb->priority]->pcbPrevious->pcbNext=pcb;
		priority_lists[pcb->priority]->pcbPrevious = pcb;
	}
}





//----------------------------------------------PARTIE APPELS SYSTEME-----------------------------------------------------

	//---------------------------------------WAIT--------------------------------
void wait(int nbQuantums)
{
	current_pcb->etatP = WAITING;
	current_pcb->nbQuantums = nbQuantums;
	ctx_switch();
}

void increment_all_waiting() //On incrémente à chaque switch
{
	struct pcb_s * pcb_temp;
	pcb_temp = current_pcb;
	
	do {		
		if(pcb_temp->etatP == WAITING && pcb_temp->pid_waiting == -1)
		{
			pcb_temp->nbQuantums--;
			if(pcb_temp->nbQuantums == 0) 
			{
				pcb_temp->etatP = READY;
			}
		}
		pcb_temp = pcb_temp->pcbNext;
	}
	while(pcb_temp != current_pcb);
}

	//--------------------------------------KILL---------------------------------
void kill(unsigned int process_id)
{
	//Chercher ID dans la boucle
	struct pcb_s* pcb_to_delete = NULL;
	if( (pcb_to_delete = findProcessById(process_id)) == NULL )
	{
		//Send error
	}else
	{

		//On le retire de la boucle
		pcb_to_delete->pcbPrevious->pcbNext = pcb_to_delete->pcbNext;
		pcb_to_delete->pcbNext->pcbPrevious = pcb_to_delete->pcbPrevious;

		//On supprime le processus
		phyAlloc_free((void *)pcb_to_delete->stack_base, pcb_to_delete->stack_size);
		phyAlloc_free(pcb_to_delete, sizeof(pcb_s));
	}

}

	//----------------------------------------WAITPID-----------------------------
void waitpid(unsigned int process_id)
{
	current_pcb->etatP = WAITING;
	current_pcb->pid_waiting = process_id;
	ctx_switch();
}





//---------------------------------------------PARTIE SWITCH-------------------------------------------------------------
struct pcb_s* elect_pcb_into_list(unsigned short priority){
	int should_execute = 0;
	pcb_s *head_pcb = priority_lists[priority];
	pcb_s *looking_pcb = head_pcb;
	if(head_pcb == NULL){
		return NULL;
	}
	do{
		looking_pcb = looking_pcb->pcbNext;
		if(looking_pcb->etatP == TERMINATED){
			pcb_s *old_pcb = looking_pcb;
			if(old_pcb->pcbNext == old_pcb){
					priority_lists[priority]=NULL;
			} else {
				looking_pcb = looking_pcb->pcbPrevious;
				// Update Next/Previous
				old_pcb->pcbPrevious->pcbNext = old_pcb->pcbNext;
				old_pcb->pcbNext->pcbPrevious = old_pcb->pcbPrevious;
			}

			restartAllWaitingPIDProcess(old_pcb->ID);

			// Free memory space reserved for deleted process
			phyAlloc_free((void *)old_pcb->stack_base, old_pcb->stack_size);
			phyAlloc_free(old_pcb, sizeof(pcb_s));
		}else if(current_pcb->pcbNext->etatP == WAITING)
		{
			// Nothing to do
		} else {
			should_execute = 1;
		}
	} while(should_execute == 0 && looking_pcb != head_pcb);
	
	if(should_execute){
		return looking_pcb;
	}
	else {
		return NULL;
	}
}	

void elect()
{
	int i;
	pcb_s* next_pcb = NULL; //Will be executed
	
	if(current_pcb != NULL && current_pcb->etatP == RUNNING){
		current_pcb->etatP = READY;
	}	
	for (i=PRIORITY_NUMBER-1; i>=0 && next_pcb==NULL; --i){
		next_pcb = elect_pcb_into_list(i);
	}
	
	// TODO No process
	current_pcb=next_pcb;
	current_pcb->etatP = RUNNING;
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
	increment_all_waiting();
	
	__asm("mov sp, %0" : : "r"(current_pcb->stack_pointer));
	__asm("pop {r0-r12}");
	
	//On arme le timer
	set_tick_and_enable_timer();
	//On dit que la suite du code est interruptible
	ENABLE_IRQ();

	// Jump -> On met la valeur de lr dans PC
	__asm("rfeia sp!");	
}
