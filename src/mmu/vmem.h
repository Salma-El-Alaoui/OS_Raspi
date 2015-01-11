#include <stdint.h>

#ifndef VMEM_H
#define VMEMC_H

unsigned int init_kern_translation_table (void);
uint8_t* vMem_Alloc(unsigned int nbPages);
void vMem_Free(uint8_t* ptr, unsigned int nbPages);

#endif
