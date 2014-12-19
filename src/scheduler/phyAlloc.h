#ifndef PHYALLOC_H
#define PHYALLOC_H

void* phyAlloc_alloc(unsigned int size);
void phyAlloc_free(void *ptr, unsigned int size);
void phyAlloc_init(char* start_heap, unsigned int heap_size);

#endif
