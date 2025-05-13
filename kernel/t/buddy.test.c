#define BUDDY_IMPL
#include "mem/buddy.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mem/page.h"

#define PAGE_SIZE               4096
#define PAGE_SHIFT              12
#define PAGE_BUDDY              (1 << 0)

#define BUDDY_MAX_LEVEL         10
#define BUDDY_SMALLEST_EXPONENT 5  // e.g., 2^5 = 32 bytes

static unsigned int mock_phys_base = 0x10000000;
#define V2P(x) ((uintptr_t)(x) - mock_phys_base)
#define P2V(x) ((uintptr_t)(x) + mock_phys_base)

static inline void
kmemset (void *p, int c, size_t n) {
  memset(p, c, n);
}

static unsigned int
kmalloc (size_t size) {
  return (unsigned int)malloc(size);
}

static void
kfree (unsigned int ptr) {
  free((void *)ptr);
}

// unsigned int blocksizes[BUDDY_MAX_LEVEL + 1];

void
test_alloc_multiple_sizes () {
  printf("[+] test_alloc_multiple_sizes\n");
  unsigned int a = buddy_malloc(32);  // smallest block
  unsigned int b = buddy_malloc(128);
  unsigned int c = buddy_malloc(512);

  // assert(a && b && c);
  // assert(a != b && b != c && a != c);

  // buddy_free(a);
  // buddy_free(b);
  // buddy_free(c);
}

void
run_buddy_tests (void) {
  buddy_init();
  test_alloc_multiple_sizes();
}
