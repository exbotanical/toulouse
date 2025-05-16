#ifndef STUBS_H
#define STUBS_H

#include <stdlib.h>

u_int32_t image_start = 0;
u_int32_t image_end   = 0;
u_int32_t text_end    = 0;
u_int32_t data_end    = 0;

unsigned int
kmalloc (size_t size) {
  return (unsigned int)malloc(size);
}

void
kfree (unsigned int ptr) {
  free((void*)ptr);
}

#endif /* STUBS_H */
