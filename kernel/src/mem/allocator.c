#include "mem/allocator.h"

#include "drivers/console/vga.h"
#include "mem/buddy.h"
#include "mem/paging.h"

unsigned int
k_malloc (size_t size) {
  // page_t      *pg;
  unsigned int max_size;
  // unsigned int addr;

  max_size = blocksizes[BUDDY_MAX_LEVEL - 1];
  if (size + sizeof(buddy_low_head_t) <= max_size) {
    size += sizeof(buddy_low_head_t);
    return buddy_low_alloc(size);
  }

  // TODO: Impl buddy_high
  if (size > PAGE_SZ) {
    vgaprintf("[WARN]: %s(): size (%d) is larger than PAGE_SZ\n", __func__, size);
    return 0;
  }

  // if ((pg = get_free_page())) {
  //   addr = pg->page << PAGE_SHIFT;
  //   return P2V(addr);
  // }

  // OOM
  return 0;
}

void
k_free (unsigned int addr) {
  struct page *pg;
  unsigned     paddr;

  paddr = V2P(addr);
  pg    = &page_pool[paddr >> PAGE_SHIFT];

  if (pg->flags & PAGE_BUDDYLOW) {
    buddy_low_free(addr);
  } else {
    // release_page(pg);
  }
}
