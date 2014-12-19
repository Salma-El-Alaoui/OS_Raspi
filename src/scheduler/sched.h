#ifndef SCHED_H
#define SCHED_H

/**
* FIXED_PRIORITY_SCHED
* OWN_SCHED
* RR_SCHED
*
* PRIORITY_SCHED
*/
#if defined FIXED_PRIORITY_SCHED  || defined OWN_SCHED
	#define PRIORITY_SCHED
#endif

typedef void (*func_t) ( void*);
/*
*pour une fonction:
*3 parametres (3 int de 32)
* 3 variables locales (3 int de 32)
*de la place pour faire les calculs (128)
*on estime qu'on veut appler 5 fonctions
=> 200 donc 256
*/
#define STACK_SIZE 256
#define PRIORITY_NUMBER 16
typedef enum etatProcessus {WAITING, READY, RUNNING, TERMINATED} etatProcessus;

typedef struct pcb_s
{
	unsigned int pid;

	unsigned int instruct_address;
	unsigned int stack_pointer;
	unsigned int stack_base;
	
	int stack_size;
	
	func_t f;
	void * args;
	
	enum etatProcessus etatP;
	int nbQuantums;
	
#if !defined RR_SCHED && !defined FIXED_PRIORITY_SCHED
	struct pcb_s * pcbNext;	//Linked list
	struct pcb_s * pcbPrevious;	//Linked list
#endif

#ifdef PRIORITY_SCHED
	unsigned short priority;
#endif

#ifdef OWN_SCHED
	struct pcb_s * pcb_left;	//Tree structure
	struct pcb_s * pcb_right;	//Tree structure
	unsigned int key;	//Tree key
	unsigned int real_priority;
#endif
	
} pcb_s;

#ifdef PRIORITY_SCHED
void create_process(func_t f, void* args, unsigned int stack_size, unsigned short priority);
#else
void create_process(func_t f, void* args, unsigned int stack_size);
#endif

void start_sched();

#endif
