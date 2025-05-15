#ifndef STUBS_H
#define STUBS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned int
kmalloc (size_t size) {
  return (unsigned int)malloc(size);
}

void
kfree (unsigned int ptr) {
  free((void*)ptr);
}

#endif /* STUBS_H */
