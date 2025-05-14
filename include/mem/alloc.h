#ifndef ALLOC_H
#define ALLOC_H

#include "lib/types.h"

unsigned int kmalloc(size_t size);
void         kfree(unsigned int);

#endif /* ALLOC_H */
