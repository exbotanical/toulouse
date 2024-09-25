#include "mem/manager.h"

#include "arch/interrupt.h"
#include "common/types.h"
#include "debug/panic.h"
#include "drivers/console/vga.h"
#include "lib/list.h"
#include "lib/math.h"
#include "lib/string.h"
#include "mem/base.h"
#include "mem/mmu.h"
#include "mem/module.h"
#include "sync/spinlock.h"

#define ADDRESS_SPACE_SZ 4294967295ULL  // TODO: wut
#define MAX_ORDER        10
#define MMAP_BUFF_SIZE   256

#define HEAP_SZ          (NUM_HEAP_PAGES * PAGE_SZ)

extern uint32_t image_start;
extern uint32_t image_end;

static phys_addr_t kernel_start;
static phys_addr_t kernel_end;
static phys_addr_t heap_start;
static phys_addr_t heap_end;

static list_head_t free_page_list[MAX_ORDER + 1];

static __attribute__((section(".init.data"))) multiboot_memory_map_t mmaps[MMAP_BUFF_SIZE];
static __attribute__((section(".init.data"))) uint32_t               mmap_length;

page_t *all_pages;

static inline bool
is_page_between_addrs (uint32_t idx, uint32_t start, uint32_t end) {
  uint32_t start_idx = div_down(start, PAGE_SZ);
  uint32_t end_idx   = div_down(end, PAGE_SZ);

  return idx >= start_idx && idx <= end_idx;
}

static inline bool
is_kernel_page (uint32_t idx) {
  return is_page_between_addrs(idx, kernel_start, kernel_end);
}

static inline bool
is_heap_page (uint32_t idx) {
  return is_page_between_addrs(idx, heap_start, heap_end);
}

static inline uint32_t
get_page_index (page_t *page) {
  return (uint32_t)page - ((uint32_t)all_pages / sizeof(page_t));
}

static inline uint32_t
get_order_idx (uint32_t idx, uint32_t order) {
  return div_down(idx, 1ULL << order);
}

static inline bool
is_master (page_t *page) {
  return !(get_order_idx(get_page_index(page), page->order - 1) % 2);
}

static inline bool
is_free (page_t *page) {
  return !(page->flags & PAGE_FLAG_USED) && !(page->flags & PAGE_FLAG_PERM);
}

static inline page_t *
get_buddy (page_t *page) {
  uint32_t todo = (1ULL << page->order);
  return is_master(page) ? page + todo : page - todo;
}

// __attribute__((pure)) ???
static inline page_t *
get_master (page_t *page) {
  return is_master(page) ? page : get_buddy(page);
}

static inline void
add_to_freelist (page_t *page) {
  list_append(&page->list, &free_page_list[page->order]);
}

static page_t *
block_join (page_t *page) {
  page_t *master = get_master(page);

  master->order++;

  return master;
}

// Start with the given block. Continue aggregating the largest free block which contains the
// original block w/buddy. Keep going as long as possible, then mark the big block as free.
static void
ripple_join (page_t *page) {
  page_t *block = page;
  page_t *buddy = get_buddy(block);

  while (is_free(buddy) && buddy->order == block->order && block->order < MAX_ORDER) {
    list_remove(&buddy->list);

    block = block_join(block);
    buddy = get_buddy(block);
  }

  add_to_freelist(block);
}

static void
free_page (page_t *page) {
  uint32_t   flags;
  spinlock_t lock;  // TODO: global
  spinlock_init(&lock);

  int_save_disable(&flags);
  spinlock_lock(&lock);

  uint32_t page_sz = 1 << page->order;
  for (uint32_t i = 0; i < page_sz; i++) {
    // ???
    page[i].flags = 0;
  }

  ripple_join(page);

  spinlock_unlock(&lock);
  int_store(flags);
}

static void
claim_page (uint32_t idx) {
  all_pages[idx].flags &= ~PAGE_FLAG_PERM;
  free_page(&all_pages[idx]);
  // pages_available++;
}

/**
 * Find and return the page index of the first available memory region with enough contiguous memory
 * to accommodate the requested `num_pages`.
 *
 * @param num_pages
 * @return phys_addr_t
 */
static phys_addr_t
find_suitable_memory (uint32_t num_pages) {
  for (uint32_t i = 0; i < mmap_length; i++) {
    multiboot_memory_map_t mmap = mmaps[i];

    if (mmap.type == MULTIBOOT_MEMORY_AVAILABLE) {
      // Skip regions that are too large
      if (mmap.addr >= ADDRESS_SPACE_SZ) {
        // TODO: debug/log
        continue;
      }

      // Get page index boundaries of the current mmap
      uint32_t start = div_up(mmap.addr, PAGE_SZ);
      uint64_t end   = div_up(mmap.addr + mmap.len, PAGE_SZ);

      // If were beyond the max page index, cap it.
      if (end > ADDRESS_SPACE_SZ / PAGE_SZ) {
        end = ADDRESS_SPACE_SZ / PAGE_SZ;
      }

      uint32_t num_pages_in_region = end - start;
      if (num_pages_in_region < num_pages) {
        continue;
      }

      uint32_t contiguous       = 0;
      uint32_t contiguous_start = 0;

      while (start < end) {
        if (!contiguous) {
          contiguous_start = start;
        }

        // If...reset contiguous count
        if (is_kernel_page(start) || is_module_page(start)) {
          contiguous = 0;
        } else {
          contiguous++;
        }

        start++;

        if (contiguous >= num_pages) {
          return contiguous_start + PAGE_SZ;
        }
      }
    }
  }

  return 0;
}

static void
claim_all_pages (void) {
  // TODO: dedupe logic
  for (uint32_t i = 0; i < mmap_length; i++) {
    multiboot_memory_map_t mmap = mmaps[i];

    if (mmap.type == MULTIBOOT_MEMORY_AVAILABLE) {
      if (mmap.addr >= ADDRESS_SPACE_SZ) {
        continue;
      }

      uint32_t start = div_up(mmap.addr, PAGE_SZ);
      uint64_t end   = div_up(mmap.addr + mmap.len, PAGE_SZ);

      if (end > ADDRESS_SPACE_SZ / PAGE_SZ) {
        end = ADDRESS_SPACE_SZ / PAGE_SZ;
      }

      for (uint32_t j = start; j < end; j++) {
        if (!is_kernel_page(j) && !is_module_page(j) && !is_heap_page(j)) {
          claim_page(j);
        }
      }
    }
  }
}

void
mm_init (multiboot_info_t *mbi) {
  for (uint32_t i = 0; i <= MAX_ORDER; i++) {
    // TODO: define_list vs list_init - make use-cases more apparent
    list_init(&free_page_list[i]);
  }

  kernel_start = (uint32_t)&image_start;
  kernel_end   = (uint32_t)&image_end;  // 20E820

  // Save the module list to a buffer
  module_init(mbi);

  // Save the mmap to a buffer as well...
  mmap_length = mbi->mmap_length;
  if (mmap_length > MMAP_BUFF_SIZE) {
    // TODO: k_panicf
    k_panic("mmap too large");
  }

  k_memcpy(mmaps, mbi->mmap, mmap_length * sizeof(multiboot_memory_map_t));

  heap_start = find_suitable_memory(NUM_HEAP_PAGES);
  if (!heap_start) {
    k_panic("could not find a sufficiently large contiguous memory region for heap");
  }
  heap_end  = heap_start + HEAP_SZ;

  // Replace the temporary page table we used on boot with a better one.
  all_pages = (page_t *)mmu_init(kernel_end, heap_start);

  // At this point, we can assume everything that wasn't kernel memory or module memory has been
  // clobbered. Henceforth, we may only use addresses for pre-allocated space.
  // ...This includes the multiboot data, so we nullify the pointer.
  mbi       = NULL;

  // for (uint32_t page = 0; page < NUM_ENTRIES * NUM_ENTRIES; page++) {
  //   all_pages[page].flags = PAGE_FLAG_PERM | PAGE_FLAG_USED;
  //   all_pages[page].order = 0;
  // }

  // claim_all_pages();
}
