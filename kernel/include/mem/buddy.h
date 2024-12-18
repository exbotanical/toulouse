#ifndef BUDDY_H
#define BUDDY_H

#include "common/types.h"

#define BUDDY_MAX_LEVEL 7

typedef struct buddy_low_head buddy_low_head_t;

struct buddy_low_head {
  unsigned char     level; /* size class (exponent of the power of 2) */
  buddy_low_head_t *prev;
  buddy_low_head_t *next;
};

void         buddy_low_init(void);
unsigned int buddy_low_alloc(size_t size);
void         buddy_low_free(unsigned int addr);

#endif /* BUDDY_H */
