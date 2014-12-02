#include "phyAlloc.h"

struct fl {
	struct fl	*next;
	unsigned int	size;
} *freelist = (struct fl *)0;

static char* top;
static char* heap_limit;

void*
phyAlloc_alloc(unsigned int size)
{
	register struct fl *f = freelist, **prev;

	prev = &freelist;
	while (f && f->size < size) {
		prev = &f->next;
		f = f->next;
	}
	if (f == (struct fl *)0) {
		f = (struct fl *)top;
		top += (size + 3) & ~3;
		/* No space available anymore */
		if (top > (char *)heap_limit)
		  return 0;
	} else
		*prev = f->next;

	return ((void *)f);
}

void
phyAlloc_free(void *ptr, unsigned int size)
{
	register struct fl *f = (struct fl *)ptr;

	f->size = (size + 3) & ~3;
	f->next = freelist;
	freelist = f;
}

void
phyAlloc_init(char* heap_start, unsigned int heap_size)
{
  top = heap_start;
  heap_limit = heap_start + heap_size;
}
