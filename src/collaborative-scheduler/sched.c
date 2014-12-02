
#include "sched.h"
#include "phyAlloc.h"
struct pcb_s * head = (struct pcb_s *)0; 
struct pcb_s * current_process = (struct pcb_s *)0;
struct pcb_s idle;//traaash

void start_current_process()
{
	current_process->pstate = READY;
	current_process->function();
	current_process->pstate = TERMINATED;
	ctx_switch();
}

void init_pcb(struct pcb_s* pcb, func_t f, void* args, unsigned int stack_size)
{
	static int pid_counter = 0;
	pcb->pid = pid_counter++;
	//Initialize process state to NEW
	pcb->pstate = NEW;
	
	//Assign function and arguments
	pcb->function = f;
	pcb->args =  args;
 
	//PC  & SP
	pcb->pc = (int*)&start_current_process;
  	pcb->sp= (int*) phyAlloc_alloc(stack_size)+ (stack_size -1)/sizeof(int)-13;

}

void create_process(func_t f, void* args, unsigned int stack_size)
{
	struct pcb_s* pcb = (struct pcb_s*)phyAlloc_alloc(sizeof(struct pcb_s));
	
	//Insert into linked list
	if (!head)//Empty list
	{
		head = pcb;
	} 
	else
	{		
		pcb->next = head->next ;
	}
 	head->next = pcb;
	//Initialize PCB
	init_pcb(pcb, f, args, stack_size);
}



void elect () 
{
	
	struct pcb_s* pcb;
	pcb = current_process;
	while (pcb->next->pstate == TERMINATED) {
	/* If no process to be executed -> note that and leave loop */
		if (pcb->next == pcb) {
			pcb =0;
			break;
		} else {
			if (pcb->next == head)
				head = pcb;
			pcb->next = pcb->next->next;

			phyAlloc_free(pcb->next,sizeof(struct pcb_s));

			pcb = pcb->next;
		}
	}

	if (pcb != 0) {
		 current_process = pcb->next;
	}
	
}

void start_sched(){

	current_process = &idle;
	current_process->next = head;
}

void __attribute__ ((naked)) ctx_switch()
{

	//Save the context
		__asm("push {r0-r12}");	
		__asm("mov %0, sp" : "=r"(current_process->sp));
		__asm("mov %0, lr" : "=r"(current_process->pc));
		
  	//elect a new process 
	elect();
	
	//restore the context of the elected process
	__asm("mov sp, %0" : : "r"(current_process->sp));
	__asm("mov lr, %0" : : "r"(current_process->pc));
	__asm("pop {r0-r12}");
	__asm("bx lr");
	
}

