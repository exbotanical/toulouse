#ifndef MEM_BUDDY_H
#define MEM_BUDDY_H

#include "lib/types.h"

#define BUDDY_MAX_LEVEL         7  // 128b
#define BUDDY_SMALLEST_EXPONENT 5  // 32b

typedef struct buddy_head buddy_head_t;

struct buddy_head {
  /**
   * The size class of the block
   * (i.e. the exponent of 2 that we're on e.g. 32, 64, ..., 512)
   */
  unsigned char level;
  buddy_head_t  next;
  buddy_head_t  prev;
};

/**
 * Initialize the buddy allocator for kernel memory. The buddy allocator is used for resources
 * smaller than `PAGE_SIZE`.
 */
void         buddy_init(void);
void         buddy_free(unsigned int addr);
unsigned int buddy_malloc(size_t size);

#endif /* MEM_BUDDY_H */
