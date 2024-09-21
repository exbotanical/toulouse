#include "mem/manager.h"

#include "common/types.h"
#include "debug/panic.h"
#include "drivers/console/vga.h"
#include "lib/list.h"
#include "lib/math.h"
#include "lib/string.h"
#include "mem/base.h"
#include "mem/module.h"

#define ADDRESS_SPACE_SZ 4294967295ULL  // TODO: wut
#define MAX_ORDER        10
#define MMAP_BUFF_SIZE   256

#define HEAP_SZ          (NUM_HEAP_PAGES * PAGE_SZ)

extern uint32_t image_start;
extern uint32_t image_end;
extern uint32_t image_start;
extern uint32_t image_end;

static phys_addr_t kernel_start;
static phys_addr_t kernel_end;
static phys_addr_t heap_start;
static phys_addr_t heap_end;

static list_head_t free_page_list[MAX_ORDER + 1];

static multiboot_memory_map_t mmaps[MMAP_BUFF_SIZE];
static uint32_t               mmap_length;

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

/**
 * Find and return the page index of the first available memory region with enough contiguous memory
 * to accommodate the requested `num_pages`.
 *
 * @param num_pages
 * @return phys_addr_t
 */
static phys_addr_t
memm_find_region (uint32_t num_pages) {
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

void
mm_init (multiboot_info_t *mbi) {
  for (uint32_t i = 0; i <= MAX_ORDER; i++) {
    // TODO: define_list vs list_init - make use-cases more apparent
    list_init(&free_page_list[i]);
  }

  // Save the module list to a buffer
  module_init(mbi);

  // Save the mmap to a buffer as well...
  mmap_length = mbi->mmap_length;
  if (mmap_length > MMAP_BUFF_SIZE) {
    // TODO: k_panicf
    k_panic("mmap too large");
  }

  k_memcpy(mmaps, mbi->mmap, mmap_length * sizeof(multiboot_memory_map_t));

  heap_start = memm_find_region(NUM_HEAP_PAGES);
  if (!heap_start) {
    k_panic("could not find a sufficiently large contiguous memory region for heap");
  }
  heap_end = heap_start + HEAP_SZ;
}
