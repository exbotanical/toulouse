#include "mem/alloc.h"

#include "lib/compiler.h"
#include "lib/string.h"
#include "lib/types.h"
#include "mem/base.h"
#include "mem/buddy.h"
#include "mem/page.h"

overridable unsigned int
kmalloc (size_t size) {
  // If the requested size can be accommodated by a buddy allocator block, use that
  size_t max_size = blocksizes[BUDDY_MAX_LEVEL - 1];
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
  if ((page = page_get_free())) {
    unsigned int addr = page->page_num << PAGE_SHIFT;
    return P2V(addr);
  }

  return 0;
}

overridable void
kfree (unsigned int addr) {
  page_t* page = &free_page_list[V2P(addr) >> PAGE_SHIFT];

  if (page->flags & PAGE_BUDDY) {
    buddy_free(page);
  } else {
    page_release(page);
  }
}
