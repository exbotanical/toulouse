#define BUDDY_IMPL
#include "buddy.c"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buddy_mock.h"

#define PAGE_SIZE               4096
#define PAGE_SHIFT              12
#define PAGE_BUDDY              (1 << 0)

#define BUDDY_MAX_LEVEL         10
#define BUDDY_SMALLEST_EXPONENT 5  // e.g., 2^5 = 32 bytes

typedef struct buddy_head {
  struct buddy_head *next;
  struct buddy_head *prev;
  int                level;
} buddy_head_t;

typedef struct {
  unsigned int flags;
} page_t;

static page_t page_pool[1024];  // Just mock 1024 pages

static unsigned int mock_phys_base = 0x10000000;
#define V2P(x) ((uintptr_t)(x) - mock_phys_base)
#define P2V(x) ((uintptr_t)(x) + mock_phys_base)

static inline void
kmemset (void *p, int c, size_t n) {
  memset(p, c, n);
}

static void *
kmalloc (size_t size) {
  return malloc(size);
}

static void
kfree (void *ptr) {
  free(ptr);
}

size_t blocksizes[BUDDY_MAX_LEVEL + 1];

void
init_blocksizes () {
  for (int i = 0; i <= BUDDY_MAX_LEVEL; i++) {
    blocksizes[i] = 1 << (i + BUDDY_SMALLEST_EXPONENT);
  }
}

void
test_alloc_free () {
  printf("Test: Alloc and free small block\n");
  unsigned int ptr = buddy_malloc(64);
  assert(ptr != 0);
  buddy_free(ptr);
}

void
run_buddy_tests (void) {
  init_blocksizes();
  // buddy_init();
  // test_alloc_
  // test_alloc_free();free();
}
