#include "mem/buddy.h"

#include "drivers/console/vga.h"
#include "lib/string.h"
#include "mem/allocator.h"
#include "mem/base.h"
#include "mem/paging.h"

static buddy_low_head_t *freelist[BUDDY_MAX_LEVEL + 1];

static buddy_low_head_t *
get_buddy (buddy_low_head_t *block) {
  int mask;

  mask = 1 << (block->level + 5);
  return (buddy_low_head_t *)((unsigned int)block ^ mask);
}

static buddy_low_head_t *
allocate (unsigned int size) {
  buddy_low_head_t *block;
  buddy_low_head_t *buddy;

  page_t      *pg;
  unsigned int addr, paddr;
  int          level;

  for (level = 0; blocksizes[level] < size; level++);

  if (level == BUDDY_MAX_LEVEL) {
    if ((addr = k_malloc(PAGE_SZ))) {
      paddr      = V2P(addr);
      pg         = &page_pool[paddr >> PAGE_SHIFT];
      pg->flags |= PAGE_BUDDYLOW;
      block      = (buddy_low_head_t *)addr;
    } else {
      vgaprintf("[WARN]: %s(): not enough memory\n", __func__);
      return NULL;
    }
    // kstat.buddy_low_num_pages++;
    block->prev = block->next = NULL;
    return block;
  }

  if (freelist[level] != NULL) {
    /* we have a block on freelist */
    block = freelist[level];
    if (block->next) {
      block->next->prev = block->prev;
    }
    // TODO: Use lib/list
    if (block->prev) {
      block->prev->next = block->next;
    }
    if (block == freelist[level]) {
      freelist[level] = block->next;
    }
  } else {
    /* split a bigger block */
    block = allocate(blocksizes[level + 1]);

    if (block != NULL) {
      /* put the buddy on the free list */
      block->level = level;
      buddy        = get_buddy(block);
      buddy->level = level;
      buddy->prev = buddy->next = NULL;
      freelist[level]           = buddy;
    }
  }

  return block;
}

static void
deallocate (buddy_low_head_t *block) {
  buddy_low_head_t **h, *buddy, *p;
  page_t            *pg;
  unsigned int       addr, paddr;
  int                level;

  level = block->level;
  buddy = get_buddy(block);

  p     = freelist[level];
  while (p != NULL) {
    if (p == buddy) {
      break;
    }
    p = p->next;
  }

  if (p == buddy) {
    /* remove buddy from its free list */
    if (buddy->next) {
      buddy->next->prev = buddy->prev;
    }
    if (buddy->prev) {
      buddy->prev->next = buddy->next;
    }
    if (buddy == freelist[level]) {
      freelist[level] = buddy->next;
    }
    /* deallocate block and its buddy as one single block */
    if (level < BUDDY_MAX_LEVEL - 1) {
      if (block > buddy) {
        buddy->level++;
        deallocate(buddy);
      } else {
        block->level++;
        deallocate(block);
      }
    }

    if (level == BUDDY_MAX_LEVEL - 1) {
      addr       = (unsigned int)block;
      paddr      = V2P(addr);
      pg         = &page_pool[paddr >> PAGE_SHIFT];
      pg->flags &= ~PAGE_BUDDYLOW;
      k_free(addr);
      // kstat.buddy_low_num_pages--;
    }
  } else {
    /* buddy not free, put block on its free list */
    h = &freelist[level];

    if (!*h) {
      *h          = block;
      block->prev = block->next = NULL;
    } else {
      block->next = *h;
      block->prev = NULL;
      (*h)->prev  = block;
      *h          = block;
    }
  }
}

void
buddy_low_init (void) {
  k_memset(freelist, 0, sizeof(freelist));
}

unsigned int
buddy_low_alloc (size_t size) {
  buddy_low_head_t *block;
  int               level;

  for (level = 0; blocksizes[level] < size; level++);

  // kstat.buddy_low_count[level]++;
  // kstat.buddy_low_mem_requested += blocksizes[level];
  block = allocate(size);
  return block ? (unsigned int)(block + 1) : 0;
}

void
buddy_low_free (unsigned int addr) {
  buddy_low_head_t *block;
  // int               level;

  block = (buddy_low_head_t *)addr;
  block--;
  // level = block->level;
  // kstat.buddy_low_count[level]--;
  // kstat.buddy_low_mem_requested -= bl_blocksize[level];
  deallocate(block);
}
