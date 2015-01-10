#include "sched.h"
#include "phyAlloc.h"
#include "hw.h"
#include <stdlib.h>


struct pcb_s * current_pcb = NULL;
struct pcb_s * trash_pcb = NULL;

#ifdef FIXED_PRIORITY_SCHED
pcb_s* priority_lists[PRIORITY_NUMBER];	//Priority Array
#elif defined OWN_SCHED
struct pcb_s * pcb_root;	//Tree root
#endif

// ----------------------------------------------------------------------------------------------------
//										UTILS
// ----------------------------------------------------------------------------------------------------
void update_waiting(struct pcb_s * pcb, void * args)
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

void restart_waiting_PID_process(struct pcb_s * pcb, void * args)
{
	unsigned int process_id = (unsigned int) args;
	if(pcb->pid_waiting == process_id)
	{
		pcb->etatP = READY;
		pcb->pid_waiting = -1;
	}
}

void waiting_loop(){
	while(1);
}

// ----------------------------------------------------------------------------------------------------
//										STRUCTURE CALLS
// ----------------------------------------------------------------------------------------------------
#ifdef OWN_SCHED
void insert_process_loop(struct pcb_s * new_process, struct pcb_s ** pcb_head) {
	if(*pcb_head == NULL){
		*pcb_head = new_process;
	}
	else{
		if(new_process->key < (*pcb_head)->key){
			insert_process_loop(new_process, &(*pcb_head)->pcb_left);
		}
		else{
			insert_process_loop(new_process, &(*pcb_head)->pcb_right);
		}
		
	}	
}

void insert_process(struct pcb_s * new_process) {
	insert_process_loop(new_process, &pcb_root);
}

/*void delete_process(struct pcb_s * old_process){
	pcb_s * temp = NULL;
	// Free
	phyAlloc_free((void *)old_process->stack_base, old_process->stack_size);

  	if (old_process->pcb_left == NULL) //no left child
  	{
  		temp = old_process->pcb_right;
		*old_process = *(old_process->pcb_right);

// TODO CHILDREN

		// Free PCB
		phyAlloc_free(temp, sizeof(pcb_s));
  	}

  	else if (old_process->pcb_right == NULL) //no right child
    {
		temp = old_process->pcb_left;
		*old_process = *(old_process->pcb_left);
		// Free PCB
		phyAlloc_free(temp, sizeof(pcb_s));
    }
  	else //left & right                           
    {
     	temp = old_process->pcb_left; //right-most node of left sub-tree
     	while (temp->pcb_right != NULL)
       		temp = temp->pcb_right; // we're there
		//copy the roots' children to the node.
		temp->pcb_right = old_process->pcb_right;
		temp->pcb_left = old_process->pcb_left ;
		//move the node to the root
		//memcpy(old_process, temp, sizeof(pcb_s));
		*old_process = *(temp);
		phyAlloc_free(temp, sizeof(pcb_s));
    }
}*/

//new delete

void delete_process_loop(struct pcb_s * process, struct pcb_s * parent)
{
	struct pcb_s* successor; //successsor of the node to delete
	struct pcb_s* successor_parent;

	//no left child (works when the node has neither a right nor a left child)
	if( process->pcb_left == NULL){
		if (parent == NULL)
			pcb_root= process->pcb_right;
		else if(process == parent->pcb_left)
			parent->pcb_left = process->pcb_right;
		else
			parent->pcb_right = process->pcb_right;
	}

	//no right child
	else if( process->pcb_right == NULL){
		if (parent == NULL)
			pcb_root= process->pcb_left;
		else if(process == parent->pcb_left)
			parent->pcb_left = process->pcb_left;
		else
			parent->pcb_right = process->pcb_left;
	}

	//two children
	else {
		successor = process->pcb_right;
		successor_parent = process;
		while( successor->pcb_left != NULL){
			successor_parent = successor;
			successor= successor->pcb_left;
		}
		if(parent==NULL)
			pcb_root = successor;
		else if(process == parent->pcb_left)
			parent->pcb_left = successor;
		else
			parent->pcb_right = successor;

		if(successor == successor_parent->pcb_left)
			successor_parent->pcb_left = successor->pcb_right;
		else
			successor_parent->pcb_right = successor->pcb_right;
		
		successor->pcb_left = process->pcb_left;
		successor->pcb_right = process->pcb_right;
	}

	phyAlloc_free((void *)process->stack_base, process->stack_size);
	phyAlloc_free(process, sizeof(pcb_s));

}

struct pcb_s * find_parent( struct pcb_s* node, struct pcb_s ** pcb_head )
{
	if (pcb_head == NULL){
		return NULL;
	} else if(((*pcb_head)->pcb_right == node) || ((*pcb_head)->pcb_left == node)){
		return (*pcb_head);
	} else {
		pcb_s * pcb = find_parent(node, &(*pcb_head)->pcb_left);
		if(pcb != NULL){
			return pcb;
		}
		pcb = find_parent(node, &(*pcb_head)->pcb_right);
		if(pcb != NULL){
			return pcb;
		}
		return NULL;
	}

}

delete_process(struct pcb_s * pcb)
{
	delete_process_loop(pcb, find_parent(pcb, &pcb_root));
}


struct pcb_s * find_process(unsigned int pid, struct pcb_s ** pcb_head){
	if (pcb_head == NULL){
		return NULL;
	} else if((*pcb_head)->pid == pid){
		return (*pcb_head);
	} else {
		pcb_s * pcb = find_process(pid, &(*pcb_head)->pcb_left);
		if(pcb != NULL){
			return pcb;
		}
		pcb = find_process(pid, &(*pcb_head)->pcb_right);
		if(pcb != NULL){
			return pcb;
		}
		return NULL;
	}

}

struct pcb_s * find_process_by_pid(unsigned int pid){
	return find_process(pid, &pcb_root);
}

void apply_function_loop(func_pcb f, void * args, pcb_s ** pcb_head){
	if (pcb_head == NULL)
		return;
	else
	{
		apply_function_loop(f, args, &(*pcb_head)->pcb_left);
		f(*pcb_head, args);
		apply_function_loop(f, args, &(*pcb_head)->pcb_right);
	}
}

void apply_function(func_pcb f, void * args){
	apply_function_loop(f, args, &pcb_root);
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

void apply_function(func_pcb f, void * args){
	int i;
	pcb_s* looking_pcb = NULL;

	if(priority_lists == NULL) {
		return;	
	}

	for(i=0;i<PRIORITY_NUMBER;i++){
		looking_pcb = priority_lists[i];
		if(looking_pcb != NULL) {
			do {
				f(looking_pcb, args);
				looking_pcb = looking_pcb->pcbNext;
			}
			while(looking_pcb != priority_lists[i]);
		}
	}
}
#endif


#ifdef RR_SCHED
void insert_process(struct pcb_s * pcb)
{
	if(current_pcb == NULL){
		current_pcb = pcb;
		pcb->pcbNext = pcb;
		pcb->pcbPrevious = pcb;
	} else {
		pcb->pcbNext = current_pcb;
		pcb->pcbPrevious = current_pcb->pcbPrevious;
		current_pcb->pcbPrevious->pcbNext=pcb;
		current_pcb->pcbPrevious = pcb;
	}
}

void delete_process(struct pcb_s * pcb){
	if(current_pcb == pcb){
		if(pcb->pcbPrevious == pcb){
			current_pcb = NULL;
		} else {
			current_pcb = pcb->pcbPrevious;
		}
	}
	pcb->pcbPrevious->pcbNext = pcb->pcbNext;
	pcb->pcbNext->pcbPrevious = pcb->pcbPrevious;
	phyAlloc_free((void *)pcb->stack_base, pcb->stack_size);
	phyAlloc_free(pcb, sizeof(pcb_s));
}

struct pcb_s * find_process_by_pid(unsigned int pid){
	if(current_pcb != NULL) {
		pcb_s* looking_pcb= current_pcb;
		do {
			if(looking_pcb->pid == pid){
				return looking_pcb;
			}
			looking_pcb = looking_pcb->pcbNext;
		} while(looking_pcb != current_pcb);
	}
	return NULL;
}

void apply_function(func_pcb f, void * args){
	if(current_pcb != NULL) {
		pcb_s* looking_pcb= current_pcb;
		do {
			f(looking_pcb, args);
			looking_pcb = looking_pcb->pcbNext;
		} while(looking_pcb != current_pcb);
	}
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

	static unsigned int id = 1;
	pcb->pid = id++;

	pcb->pid_waiting = -1;

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
#ifdef PRIORITY_SCHED
	pcb->key = pcb->priority * 10;
#endif
}

pcb_s * create_process_priority(func_t f, void* args, unsigned int stack_size, unsigned short priority) {
	pcb_s * pcb = phyAlloc_alloc(sizeof(pcb_s));	
	init_pcb(pcb,f,args,stack_size, priority);
	insert_process(pcb);
	return pcb;
}

void start_sched()
{
	trash_pcb =  phyAlloc_alloc(sizeof(pcb_s));	
	init_pcb(trash_pcb, waiting_loop, NULL, STACK_SIZE, 0);
#ifdef RR_SCHED
	trash_pcb->pcbNext = current_pcb;
#endif
	current_pcb = trash_pcb;

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
		unsigned int pcb_pid = pcb->pid;
		delete_process(pcb);
		apply_function(restart_waiting_PID_process, (void *)pcb_pid);
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
	struct pcb_s* looking_pcb = pcb_root;
	struct pcb_s* parent = NULL;
	int found = 0;
	while (found !=1 && looking_pcb != NULL){
		while(looking_pcb->pcb_right!=NULL){
			parent = looking_pcb;
			looking_pcb = looking_pcb->pcb_right;
		}
		if(should_elect(looking_pcb)){
			found=1;
		}			
		else
			looking_pcb=parent->pcb_left;	
	}
	
	next_pcb = looking_pcb;

#endif
	if(next_pcb == NULL){
		current_pcb = trash_pcb;
	} else {
		current_pcb = next_pcb;
	}
	current_pcb->etatP = RUNNING;

	apply_function(update_waiting, NULL);

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

// TODO Check
void kill(unsigned int process_id)
{
	//Chercher ID dans la boucle
	struct pcb_s* pcb_to_delete = NULL;
	if( (pcb_to_delete = find_process_by_pid(process_id)) == NULL ) {
		//Send error
	} else {
		//On supprime le processus
		delete_process(pcb_to_delete);
	}
}

void waitpid(unsigned int process_id)
{
	current_pcb->etatP = WAITING;
	current_pcb->pid_waiting = process_id;
	ctx_switch();
}
