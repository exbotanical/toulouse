#ifndef STUBS_H
#define STUBS_H

#include <stdlib.h>

volatile char image_start[] = {0};
volatile char image_end[]   = {0};
volatile char text_end[]    = {0};
volatile char data_end[]    = {0};

unsigned int
kmalloc (size_t size) {
  return (unsigned int)malloc(size);
}

void
kfree (unsigned int ptr) {
  free((void*)ptr);
}

#endif /* STUBS_H */
