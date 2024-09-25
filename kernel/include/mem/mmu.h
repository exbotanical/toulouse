#ifndef MMU_H
#define MMU_H

#include "common/compiler.h"
#include "common/types.h"
#include "mem/base.h"

#define MMU_FLAG_PRESENT  (1 << 0)
#define MMU_FLAG_WRITABLE (1 << 1)
#define MMU_FLAG_USER     (1 << 2)

typedef struct {
  uint32_t flags     : 12;
  uint32_t phys_addr : 20;
} packed page_dirent_t;

typedef struct {
  page_dirent_t e[NUM_ENTRIES];
} packed page_dir_t;

typedef struct {
  uint32_t flags     : 12;
  uint32_t phys_addr : 20;
} packed page_tableent_t;

typedef struct {
  page_tableent_t e[NUM_ENTRIES];
} packed page_table_t;

uint32_t mmu_init(phys_addr_t kernel_end, phys_addr_t heap_start);
uint32_t map_pages(phys_addr_t phys_addr, uint32_t pages);
uint32_t map_page(phys_addr_t phys_addr);

#endif /* MMU_H */
