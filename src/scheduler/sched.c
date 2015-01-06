#include "sched.h"
#include "phyAlloc.h"
#include "hw.h"
#include <stdlib.h>


struct pcb_s * current_pcb = NULL;

#ifdef RR_SCHED
pcb_s * firstTime_pcb;
#elif defined FIXED_PRIORITY_SCHED
pcb_s* priority_lists[PRIORITY_NUMBER];	//Priority Array
#elif defined OWN_SCHED
struct pcb_s * pcb_root;	//Tree root
#endif

// ----------------------------------------------------------------------------------------------------
//										UTILS
// ----------------------------------------------------------------------------------------------------
void update_waiting(struct pcb_s * pcb)
{
	if(pcb->etatP == WAITING)
	{
		pcb->nbQuantums--;
		if(pcb->nbQuantums == 0) 
		{
			pcb->etatP = READY;
		}
	}
}


// ----------------------------------------------------------------------------------------------------
//										STRUCTURE CALLS
// ----------------------------------------------------------------------------------------------------
#ifdef OWN_SCHED
void insert_process_loop(struct pcb_s * new_process, struct pcb_s ** pcb_head) {
	//TODO
}

void insert_process(struct pcb_s * new_process) {
	//TODO
}

void delete_process_loop(struct pcb_s * old_process, struct pcb_s ** pcb_head){
	//TODO
}

void delete_process(struct pcb_s * old_process){
	//TODO
}

struct pcb_s * find_process(unsigned int pid, struct pcb_s ** pcb_head){
	//TODO
	return NULL;
}

struct pcb_s * find_process_by_pid(unsigned int pid){
	//TODO
	return NULL;
}

void apply_function(func_pcb f){
	//TODO
}

#endif

#ifdef FIXED_PRIORITY_SCHED
void insert_process(struct pcb_s * pcb)
{
	int i;
	// Init array if NULL
	if(priority_lists == NULL) {
		for(i=0;i<PRIORITY_NUMBER;i++){
			priority_lists[i] = NULL;
		}
	}

	if(priority_lists[pcb->priority] == NULL){	//if empty list
		priority_lists[pcb->priority] = pcb;
		pcb->pcbNext = pcb;
		pcb->pcbPrevious = pcb;
	} else {
		pcb->pcbNext = priority_lists[pcb->priority];
		pcb->pcbPrevious = priority_lists[pcb->priority]->pcbPrevious;
		priority_lists[pcb->priority]->pcbPrevious->pcbNext=pcb;
		priority_lists[pcb->priority]->pcbPrevious = pcb;
	}
}

void delete_process(struct pcb_s * pcb){
	pcb_s *old_pcb = pcb;
	if(old_pcb->pcbNext == old_pcb){
			priority_lists[pcb->priority]=NULL;
	} else {
		// Update Next/Previous
		old_pcb->pcbPrevious->pcbNext = old_pcb->pcbNext;
		old_pcb->pcbNext->pcbPrevious = old_pcb->pcbPrevious;
	}
	// Free memory space reserved for deleted process
	phyAlloc_free((void *)old_pcb->stack_base, old_pcb->stack_size);
	phyAlloc_free(old_pcb, sizeof(pcb_s));

}

struct pcb_s * find_process_by_pid(unsigned int pid){
	int i;
	pcb_s* pcb = NULL;

	if(priority_lists == NULL) {
		return NULL;	
	}

	for(i=0;i<PRIORITY_NUMBER;i++){
		pcb = priority_lists[i];
		if(pcb != NULL) {
			do {
				if(pcb->pid == pid) {
					return pcb;
				}
				pcb = pcb->pcbNext;
			}
			while(pcb != priority_lists[i]);
		}
	}
	return NULL;
}

void apply_function(func_pcb f){
	int i;
	pcb_s* pcb = NULL;

	if(priority_lists == NULL) {
		return;	
	}

	for(i=0;i<PRIORITY_NUMBER;i++){
		pcb = priority_lists[i];
		if(pcb != NULL) {
			do {
				f(pcb);
			}
			while(pcb != priority_lists[i]);
		}
	}

}

#endif


#ifdef RR_SCHED
void insert_process(struct pcb_s * pcb)
{
	//TODO
}

void delete_process(struct pcb_s * pcb){
	//TODO
}

struct pcb_s * find_process_by_pid(unsigned int pid){
	//TODO
	return NULL;
}

void apply_function(func_pcb f){
	//TODO
}
#endif

// ----------------------------------------------------------------------------------------------------
//										INITIALISATION
// ----------------------------------------------------------------------------------------------------

void start_current_process()
{
	current_pcb->etatP= RUNNING;
	current_pcb->f(current_pcb->args);
	current_pcb->etatP = TERMINATED;
	while(1);
    //ctx_switch();
}

void init_pcb(struct pcb_s * pcb,func_t f, void* args, unsigned int stack_size, unsigned short priority) {
//	pcb->instruct_address = (unsigned int) &start_current_process;
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
	
#ifdef PRIORITY_SCHED
	if(priority>=PRIORITY_NUMBER){
		pcb->priority= PRIORITY_NUMBER-1;
	} else {
		pcb->priority= priority;
	}
#endif

}

void create_process_priority(func_t f, void* args, unsigned int stack_size, unsigned short priority) {
	pcb_s * pcb = phyAlloc_alloc(sizeof(pcb_s));	
	init_pcb(pcb,f,args,stack_size, priority);
	insert_process(pcb);
}

void start_sched()
{
#ifdef RR_SCHED
	firstTime_pcb->pcbNext = current_pcb;
	current_pcb = firstTime_pcb;
#endif
	//On arme le timer
	set_tick_and_enable_timer();
	//On dit que la suite du code est interruptible
	ENABLE_IRQ();
}

void create_process(func_t f, void* args, unsigned int stack_size) {
	create_process_priority(f, args, stack_size,0);
}


// ----------------------------------------------------------------------------------------------------
//										SWITCH
// ----------------------------------------------------------------------------------------------------

int should_elect(struct pcb_s * pcb){
	if(pcb->etatP == TERMINATED) {
		delete_process(pcb);
	}else if(pcb->etatP == WAITING) {
		// Nothing to do
	} else {
		return 1;
	}
	return 0;
}

#ifdef FIXED_PRIORITY_SCHED
struct pcb_s* elect_pcb_into_list(unsigned short priority){
	int should_execute = 0;	
	pcb_s *head_pcb = priority_lists[priority];
	pcb_s *looking_pcb = head_pcb;
	if(head_pcb == NULL){	// TODO Put it in the loop ????? (Delete)
		return NULL;
	}
	do{
		looking_pcb = looking_pcb->pcbNext;
		should_execute = should_elect(looking_pcb);
	} while(should_execute == 0 && looking_pcb != head_pcb);
	if(should_execute){
		return looking_pcb;
	}
	return NULL;
}
#endif

void elect()
{
	pcb_s* next_pcb = NULL; //Will be executed / Iterator
	pcb_s* tmp_del_pcb = NULL; //Delete Case
	
	if(current_pcb != NULL && current_pcb->etatP == RUNNING){
		current_pcb->etatP = READY;
	}
#ifdef RR_SCHED
	tmp_del_pcb = current_pcb->pcbNext;
	do{
		next_pcb = tmp_del_pcb;
		tmp_del_pcb = next_pcb->pcbNext;
	} while(!should_elect(next_pcb));
#elif defined FIXED_PRIORITY_SCHED
	int i;
	for (i=PRIORITY_NUMBER-1; i>=0 && next_pcb==NULL; --i){
		next_pcb = elect_pcb_into_list(i);
	}
#elif defined OWN_SCHED

#endif
	
	current_pcb=next_pcb;
	current_pcb->etatP = RUNNING;

	apply_function(update_waiting);

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
	// ENABLE_IRQ(); Uselesssssssss !

	// Jump -> On met la valeur de lr dans PC
	__asm("rfeia sp!");

	
}


// ----------------------------------------------------------------------------------------------------
//										SYSCALLS
// ----------------------------------------------------------------------------------------------------
void wait(int nbQuantums)
{
	current_pcb->etatP = WAITING;
	current_pcb->nbQuantums = nbQuantums;
	ctx_switch();
}

