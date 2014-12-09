#ifndef SCHED_H
#define SCHED_H

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
	// Un peu comme un contexte
	unsigned int instruct_address;
	unsigned int stack_pointer;
	unsigned int stack_base;
	
	int stack_size;
	
	func_t f;
	void * args;
	
	enum etatProcessus etatP;
	
	// Systeme collabo : chaîne circulaire
	struct pcb_s * pcbNext;
	struct pcb_s * pcbPrevious;

//#ifdef PRIORITY_SCHED
	// Système de priorité de processus
	unsigned short priority;
//#endif

}pcb_s;

void create_process(func_t f, void* args, unsigned int stack_size, unsigned short priority);

void start_sched();

#endif
