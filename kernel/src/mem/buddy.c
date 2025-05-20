#include "mem/buddy.h"

#include "drivers/dev/char/tmpcon.h"
#include "lib/compiler.h"
#include "lib/string.h"
#include "mem/alloc.h"
#include "mem/base.h"
#include "mem/page.h"

static buddy_head_t *freelist[BUDDY_MAX_LEVEL + 1];

/**
 * Retrieves the address of the block's buddy.
 *
 * Here's how the addressing works:
 * Each address sets a level bit based on its level (i.e. exponent, where 0 -> 32, 1 -> 64, ...).
 * Buddies' addresses are complements of each other on the level bit, meaning we find the address of
 * a buddy by XORing the level bit of the block.
 */
static buddy_head_t *
buddy_get (buddy_head_t *block) {
  // Figure out which bit we need to XOR to compute the buddy address.
  int mask = 1 << (block->level + BUDDY_SMALLEST_EXPONENT);
  // XOR the level bit
  return (buddy_head_t *)((unsigned int)block ^ mask);
}

static void
buddy_remove_from_freelist (buddy_head_t *block) {
  if (block->next) {
    // TODO: Use common list
    block->next->prev = block->prev;
  }

  if (block->prev) {
    block->prev->next = block->next;
  }

  // TODO: ensure level
  if (block == freelist[block->level]) {
    freelist[block->level] = block->next;
  }
}

/**
 * Given a block, deallocates that block by coalescing it into its buddy until merged into the
 * largest available block size.
 */
static void
buddy_dealloc (buddy_head_t *block) {
  int           level = block->level;
  buddy_head_t *buddy = buddy_get(block);

  // Find the block's buddy in the freelist
  buddy_head_t *p     = freelist[level];
  while (p) {
    if (p == buddy) {
      break;
    }
    p = p->next;
  }

  // Remove buddy from its free list
  if (p == buddy) {
    buddy_remove_from_freelist(buddy);

    // If the block can be merged to be larger...
    if (level < BUDDY_MAX_LEVEL - 1) {
      // If the buddy is the lower address
      // (we address a block pair using the lower/start address, obviously)
      if (block > buddy) {
        buddy->level++;
        buddy_dealloc(buddy);
      } else {
        // Otherwise, the block is the lower half so we start there
        block->level++;
        buddy_dealloc(block);
      }
    }

    // If we're the highest supported level, we actually free the page
    if (level == BUDDY_MAX_LEVEL - 1) {
      unsigned int addr       = (unsigned int)block;
      unsigned int phys_addr  = V2P(addr);
      page_t      *page       = &free_page_list[phys_addr >> PAGE_SHIFT];
      // Mark page as no longer belonging to a buddy
      page->flags            &= ~PAGE_BUDDY;
      kfree(addr);
    }
  }
  // The buddy isn't free, so we can't merge.
  // Instead, we just place the block back on the free list.
  else {
    buddy_head_t **p2 = &freelist[level];
    // If no list already, make the block the head of the a list
    if (!*p2) {
      *p2         = block;
      block->prev = block->next = NULL;
    }
    // Otherwise, prepend it to the list
    else {
      block->next = *p2;
      block->prev = NULL;
      (*p2)->prev = block;
      *p2         = block;
    }
  }
}

static buddy_head_t *
buddy_alloc (size_t size) {
  buddy_head_t *block;

  int level;
  for (level = 0; blocksizes[level] < size; level++);

  // If we're dealing with the max level, we need to allocate another page
  if (level == BUDDY_MAX_LEVEL) {
    unsigned int addr;
    if ((addr = (unsigned int)kmalloc(PAGE_SIZE))) {
      unsigned int phys_addr  = V2P(addr);
      page_t      *page       = &free_page_list[phys_addr >> PAGE_SHIFT];
      // Mark page as belonging to a buddy
      page->flags            |= PAGE_BUDDY;
      block                   = (buddy_head_t *)addr;
    } else {
      // TODO: log err
      return NULL;
    }

    block->prev = block->next = NULL;
    return block;
  }

  // If we have a block on the freelist, use it
  if (freelist[level] != NULL) {
    block = freelist[level];
    buddy_remove_from_freelist(block);
  }
  // Otherwise, we need to split something
  else {
    block = buddy_alloc(blocksizes[level + 1]);

    // Assuming we allocated a block, put its buddy on the freelist
    if (!block) {
      block->level        = level;

      buddy_head_t *buddy = buddy_get(block);
      buddy->level        = level;
      buddy->prev = buddy->next = NULL;
      freelist[level]           = buddy;
    }
  }

  return block;
}

overridable unsigned int
buddy_malloc (size_t size) {
  buddy_head_t *block = buddy_alloc(size);
  // Increment the pointer by 1. This tells the compiler to increment the address by
  // sizeof(buddy_head_t), which puts us past the block metadata and in a region where user data can
  // go.
  // TODO: Check block metadata ALWAYS under 32b
  return block ? (unsigned int)(block + 1) : 0;
}

overridable void
buddy_free (unsigned int addr) {
  buddy_head_t *block = (buddy_head_t *)addr;
  block--;
  buddy_dealloc(block);
}

void
buddy_init (void) {
  kmemset(freelist, 0, sizeof(freelist));
}
