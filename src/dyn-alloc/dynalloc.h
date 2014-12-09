#ifndef DYNALLOC_H
#define DYNALLOC_H

typedef struct heap_bloc
{

	heap_bloc* nextBloc;
	unsigned int blocSize;
	void* blocSpace;

}heap_bloc;

heap_bloc* first_bloc;

void init_heap(unsigned int size);

void *gmalloc (unsigned size) ;
void gfree (void *ptr) ;

#endif