#include "dynalloc.h"
#include <stdlib.h>


void init_heap(unsigned int size)
{
	heap_bloc* bloc = NULL;
	bloc->blocSize = size;
	bloc->nextBloc = bloc;
	first_bloc = bloc;
	

}

void * gmalloc(unsigned size)
{
	heap_bloc* iterating_bloc = first_bloc;
	do{
			//Search for good bloc
		if(iterating_bloc->blocSize >= size)
		{
			//On crée le bloc résultant de la fragmentation du grand bloc
			heap_bloc* new_bloc;
			new_bloc->blocSize = iterating_bloc->blocSize - size;

			//On alloue notre bloc
			iterating_bloc->blocSize = size;

			//On insère le new_bloc après
			new_bloc->nextBloc = iterating_bloc->nextBloc;
			iterating_bloc->nextBloc = new_bloc;

			return iterating_bloc;
			
		}

		iterating_bloc = iterating_bloc->nextBloc;

	}
	while(iterating_bloc != first_bloc);
	
	return NULL;

}

void gfree(void *ptr)
{

}