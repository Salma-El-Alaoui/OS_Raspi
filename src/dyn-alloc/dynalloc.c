#include "dynalloc.h"


void init_heap(unsigned int size)
{
	heap_bloc* bloc = NULL;
	bloc->blocSize = size;
	bloc->blocSpace = NULL;

}

void * gmalloc(unsigned size)
{

}

void gfree(void *ptr)
{

}