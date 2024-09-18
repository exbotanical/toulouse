#ifndef MEM_BASE_H
#define MEM_BASE_H

#include "common/types.h"
#include "lib/math.h"

#define NUM_ENTRIES    1024
#define PAGE_SZ        4096

#define NUM_HEAP_PAGES div_up(sizeof(page_t) * NUM_ENTRIES * NUM_ENTRIES, PAGE_SZ)

typedef struct {
  uint32_t addr;
  uint8_t  flags;
} page_t;

typedef uint32_t phys_addr_t;

#endif /* MEM_BASE_H */
