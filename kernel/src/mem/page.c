#include "mem/page.h"

#include "arch/interrupt.h"
#include "debug/panic.h"
#include "init/bios.h"
#include "kconfig.h"
#include "kernel.h"
#include "kstat.h"
#include "lib/compiler.h"
#include "lib/string.h"
#include "mem/base.h"
#include "proc/sleep.h"

#define NUM_CACHED_PAGES (page_cache_size / sizeof(unsigned int))
#define PAGE_CACHE_HASH(inode, offset) \
  (((unsigned int)(inode) ^ (unsigned int)(offset)) % (NUM_CACHED_PAGES))

page_t *free_page_list_head;

static inline void
flag_as_kreserved (page_t *page) {
  page->flags = PAGE_RESERVED;
  kstat.kernel_reserved++;
}

static inline void
flag_as_reserved (page_t *page) {
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
insert_into_free_list (page_t *pg) {
  if (!free_page_list_head) {
    pg->prev_free = pg->next_free = pg;
    free_page_list_head           = pg;
  } else {
    pg->next_free                             = free_page_list_head;
    pg->prev_free                             = free_page_list_head->prev_free;
    free_page_list_head->prev_free->next_free = pg;
    free_page_list_head->prev_free            = pg;
  }

  kstat.num_free_pages++;
}

static void
remove_from_free_list (page_t *page) {
  if (!kstat.num_free_pages) {
    return;
  }

  page->prev_free->next_free = page->next_free;
  page->next_free->prev_free = page->prev_free;
  kstat.num_free_pages--;

  if (page == free_page_list_head) {
    free_page_list_head = page->next_free;
  }

  if (!kstat.num_free_pages) {
    free_page_list_head = NULL;
  }
}

static void
insert_into_cache (page_t *page) {
  int      i    = PAGE_CACHE_HASH(page->inode, page->file_offset);
  page_t **head = &page_cache[i];

  if (!*head) {
    *head              = page;
    (*head)->prev_hash = (*head)->next_hash = NULL;
  } else {
    page->prev_hash    = NULL;
    page->next_hash    = *head;
    (*head)->prev_hash = page;
    *head              = page;
  }

  kstat.page_cache_consumption += (PAGE_SIZE / 1024);
}

static void
remove_from_cache (page_t *page) {
  if (!page->inode) {
    return;
  }

  int      i    = PAGE_CACHE_HASH(page->inode, page->file_offset);
  page_t **head = &page_cache[i];

  while (*head) {
    if (*head == page) {
      if ((*head)->next_hash) {
        (*head)->next_hash->prev_hash = (*head)->prev_hash;
      }

      if ((*head)->prev_hash) {
        (*head)->prev_hash->next_hash = (*head)->next_hash;
      }

      if (head == &page_cache[i]) {
        *head = (*head)->next_hash;
      }

      kstat.page_cache_consumption -= (PAGE_SIZE / 1024);
      break;
    }

    head = &(*head)->next_hash;
  }
}

overridable page_t *
page_get_free (void) {
  // If the number of pages is too low, we need to reclaim some memory from the buffer cache
  if (kstat.num_free_pages <= kstat.min_free_pages) {
    // wakeup(&kswapd); // TODO:
    if (!kstat.num_free_pages) {
      sleep(func_as_ptr((void (*)(void))page_get_free), PROC_UNINTERRUPTIBLE);

      if (!kstat.num_free_pages && !kstat.pages_reclaimed) {
        // We're for sure out of memory at this point
        // TODO: log
        return NULL;
      }
    }

    kstat.min_free_pages -= NUM_BUFFER_RECLAIM;
    kstat.min_free_pages  = kstat.min_free_pages < 0 ? 0 : kstat.min_free_pages;
  } else {
    // Recalculate if free mem back to normal levels
    if (!kstat.min_free_pages) {
      if (kstat.num_free_pages > NUM_BUFFER_RECLAIM) {
        kstat.min_free_pages = (kstat.total_mem_pages * FREE_PAGES_RATIO) / 100;
      }
    }
  }

  INTERRUPTS_OFF();

  page_t *page;
  if (!(page = free_page_list_head)) {
    // TODO: log
    INTERRUPTS_ON();
    return NULL;
  }

  remove_from_free_list(page);
  remove_from_cache(page);

  page->usage_count = 1;
  page->inode       = 0;
  page->file_offset = 0;
  page->dev         = 0;

  INTERRUPTS_ON();

  return page;
}

overridable void
page_release (page_t *page) {
  if (!page_is_valid(page->page_num)) {
    kpanic("Missing page %d (0x%x).\n", page->page_num, page->page_num);
  }

  if (!page->usage_count) {
    // TODO:warn
    return;
  }

  if (--page->usage_count > 0) {
    return;
  }

  INTERRUPTS_OFF();

  insert_into_free_list(page);

  page->flags &= PAGE_RESERVED;

  // If the page isn't cached, place it at the head of the free pages list
  if (!page->inode) {
    free_page_list_head = page;
  }

  INTERRUPTS_ON();

  // Wait for free pages to be far greater than NUM_BUFFER_RECLAIM, else `page_get_free` could run
  // out of pages again and kill the process prematurely
  if (kstat.num_free_pages > (NUM_BUFFER_RECLAIM * 3)) {
    wakeup(func_as_ptr((void (*)(void))page_get_free));
  }
}

void
page_init (unsigned int num_pages) {
  kmemset(free_page_list, 0, free_page_list_size);
  kmemset(page_cache, 0, page_cache_size);

  for (unsigned int n = 0; n < num_pages; n++) {
    page_t *page      = &free_page_list[n];
    page->page_num    = n;

    // Flag the kernel pages as reserved
    unsigned int addr = n << PAGE_SHIFT;
    if (addr >= KERNEL_PHYSICAL_BASE && addr < V2P(real_last_addr)) {
      flag_as_kreserved(page);
      continue;
    }

    // Reserve a page for the kernel stack
    if (addr == 0x0000F000) {
      flag_as_reserved(page);
      continue;
    }

    // Flag special memory addresses e.g. VGA, BIOS, etc as reserved
    if (!bios_mmap_has_addr(addr)) {
      flag_as_reserved(page);
      continue;
    }

    // Everything else can be inserted into the free page list
    page->data = (char *)P2V(addr);
    insert_into_free_list(page);
  }

  kstat.total_mem_pages = kstat.num_free_pages;
  kstat.min_free_pages  = (kstat.total_mem_pages * FREE_PAGES_RATIO) / 100;
}
