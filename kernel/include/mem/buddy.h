#ifndef MEM_BUDDY_H
#define MEM_BUDDY_H

#include "common/types.h"
#include "lib/list.h"

#define BUDDY_MAX_LEVEL 7

typedef struct kbuddy_head kbuddy_head_t;

struct kbuddy_head {
  /**
   * size class (exponent of the power of 2)
   */
  unsigned char level;
  list_head_t   list_ref;
};

/**
 * Initialize the buddy allocator for kernel memory. The buddy allocator is used for resources
 * smaller than `PAGE_SIZE`.
 */
void kbuddy_init(void);

#endif /* MEM_BUDDY_H */
