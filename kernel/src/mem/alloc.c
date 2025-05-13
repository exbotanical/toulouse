#include "mem/alloc.h"

#include "lib/string.h"
#include "mem/buddy.h"
#include "mem/page.h"
#include "types.h"

unsigned int
kmalloc (size_t size) {
  // If the requested size can be accommodated by a buddy allocator block, use that
  int max_size = blocksizes[BUDDY_MAX_LEVEL - 1];
  if (size + sizeof(buddy_head_t) <= max_size) {
    size += sizeof(buddy_head_t);
    return buddy_malloc(size);
  }

  // Otherwise, we'll need to just allocate a page

  // TODO: Fix this
  if (size > PAGE_SIZE) {
    return 0;
  }

  page_t* page;
  // if ((page = page_g))
}

void
kfree (unsigned int) {}
