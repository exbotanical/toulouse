#ifndef MANAGER_H
#define MANAGER_H

#include "init/multiboot.h"
#include "lib/list.h"
#include "lib/math.h"
#include "mem/base.h"

#define PAGE_FLAG_USED  (1 << 0)
#define PAGE_FLAG_PERM  (1 << 1)
#define PAGE_FLAG_USER  (1 << 2)
#define PAGE_FLAG_CACHE (1 << 3)

// TODO: Find a central location for paging stuff (rn page constants are all over the mem/ headers)
#define NUM_HEAP_PAGES  div_up(sizeof(page_t) * NUM_ENTRIES * NUM_ENTRIES, PAGE_SZ)

typedef struct {
  uint32_t    addr;
  uint8_t     flags;
  uint8_t     order;
  uint32_t    compound_num;
  list_head_t list;
} page_t;

void mm_init(multiboot_info_t *mbi);

#endif /* MANAGER_H */
