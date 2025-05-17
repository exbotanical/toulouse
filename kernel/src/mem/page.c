#include "mem/page.h"

#include "debug/panic.h"
#include "init/bios.h"
#include "kconfig.h"
#include "kernel.h"
#include "kstat.h"
#include "lib/string.h"
#include "mem/base.h"

page_t *page_pool_head;

static inline void
page_flag_kreserved (page_t *page) {
  page->flags = PAGE_RESERVED;
  kstat.kernel_reserved++;
}

static inline void
page_flag_reserved (page_t *page) {
  page->flags = PAGE_RESERVED;
  kstat.physical_reserved++;
}

static inline unsigned int
page_mem_assign (unsigned int size, void **ptr, char *id) {
  unsigned int aligned_size = PAGE_ALIGN(size);
  if (!bios_mmap_has_addr(V2P(real_last_addr) + aligned_size)) {
    kpanic("Not enough memory for %s\n", id);
  }
  *ptr            = (void *)real_last_addr;
  real_last_addr += aligned_size;

  return aligned_size;
}

static void
page_free_list_insert (page_t *pg) {
  if (!page_pool_head) {
    pg->prev_free = pg->next_free = pg;
    page_pool_head                = pg;
  } else {
    pg->next_free                        = page_pool_head;
    pg->prev_free                        = page_pool_head->prev_free;
    page_pool_head->prev_free->next_free = pg;
    page_pool_head->prev_free            = pg;
  }

  kstat.free_pages++;
}

void
page_init (unsigned int num_pages) {
  kmemset(page_pool, 0, page_pool_size);
  kmemset(page_hash_table, 0, page_hash_table_size);

  for (unsigned int n = 0; n < num_pages; n++) {
    page_t *page      = &page_pool[n];
    page->page_num    = n;

    // Flag the kernel pages as reserved
    unsigned int addr = n << PAGE_SHIFT;
    if (addr >= KERNEL_PHYSICAL_BASE && addr < V2P(real_last_addr)) {
      page_flag_kreserved(page);
      continue;
    }

    // Reserve a page for the kernel stack
    if (addr == 0x0000F000) {
      page_flag_reserved(page);
      continue;
    }

    // Flag special memory addresses e.g. VGA, BIOS, etc as reserved
    if (!bios_mmap_has_addr(addr)) {
      page_flag_reserved(page);
      continue;
    }

    // Everything else can be inserted into the free page list
    page->data = (char *)P2V(addr);
    page_free_list_insert(page);
  }

  kstat.total_mem_pages = kstat.free_pages;
  kstat.min_free_pages  = (kstat.total_mem_pages * FREE_PAGES_RATIO) / 100;
}
