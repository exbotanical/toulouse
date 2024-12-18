#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include "common/types.h"

unsigned int k_malloc(size_t size);
void         k_free(unsigned int addr);

#endif /* ALLOCATOR_H */
