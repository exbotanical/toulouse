#include "mem/alloc.h"

#include <stdlib.h>

#include "lib/string.h"
#include "lib/types.h"
#include "libtap/libtap.h"
#include "mem/base.h"
#include "mem/page.h"

// --- Test Setup Mocks ---

#define PAGE_POOL_SIZE 128

page_t  fake_page_pool[PAGE_POOL_SIZE];
page_t* page_pool                     = fake_page_pool;

unsigned int fake_addr                = 0xDEADBEEF;
unsigned int buddy_malloc_called_with = 0;
unsigned int buddy_malloc_return_val  = 0;
unsigned int page_release_called      = 0;
unsigned int buddy_free_called        = 0;
page_t*      buddy_free_page          = NULL;

unsigned int __attribute__((weak))
buddy_malloc (size_t size) {
  buddy_malloc_called_with = size;
  return buddy_malloc_return_val;
}

void __attribute__((weak))
buddy_free (page_t* page) {
  buddy_free_called++;
  buddy_free_page = page;
}

page_t* __attribute__((weak))
page_get_free (void) {
  for (int i = 0; i < PAGE_POOL_SIZE; ++i) {
    if (!(fake_page_pool[i].flags & 0x1)) {
      fake_page_pool[i].page_num = i;
      return &fake_page_pool[i];
    }
  }
  return NULL;
}

void __attribute__((weak))
page_release (page_t* page) {
  page_release_called++;
  page->flags |= 0x1;
}

// --- Helper Macros ---

#define RESET_STATE()                                \
  memset(fake_page_pool, 0, sizeof(fake_page_pool)); \
  buddy_malloc_called_with = 0;                      \
  buddy_malloc_return_val  = 0;                      \
  page_release_called      = 0;                      \
  buddy_free_called        = 0;                      \
  buddy_free_page          = NULL;

// --- Tests ---

static void
kmalloc_uses_buddy_if_small_enough_test (void) {
  RESET_STATE();
  size_t requested        = 64;
  buddy_malloc_return_val = 0xCAFEBABE;

  unsigned int result     = kmalloc(requested);

  eq_num(result, 0xCAFEBABE, "kmalloc should delegate to buddy_malloc and return its value");
  ok(
    buddy_malloc_called_with > requested,
    "kmalloc should request more than requested size (for header)"
  );
}

static void
kmalloc_returns_0_if_too_large_test (void) {
  RESET_STATE();
  size_t too_large    = PAGE_SIZE + 1;

  unsigned int result = kmalloc(too_large);

  eq_num(result, 0, "kmalloc should return 0 if size > PAGE_SIZE");
}

static void
kmalloc_falls_back_to_page_allocator_test (void) {
  RESET_STATE();
  size_t large_size = PAGE_SIZE;

  page_t* p         = &fake_page_pool[42];
  memset(p, 0, sizeof(page_t));
  p->page_num         = 42;

  unsigned int result = kmalloc(large_size);

  eq_num(
    result,
    P2V(42 << PAGE_SHIFT),
    "kmalloc should fallback to page allocator for large allocations"
  );
}

static void
kmalloc_returns_0_if_no_pages_available_test (void) {
  RESET_STATE();
  size_t large_size = PAGE_SIZE;

  // mark all pages used
  for (int i = 0; i < PAGE_POOL_SIZE; ++i) {
    fake_page_pool[i].flags = 0x1;
  }

  unsigned int result = kmalloc(large_size);
  eq_num(result, 0, "kmalloc should return 0 if no pages are available");
}

static void
kfree_calls_buddy_free_for_buddy_page_test (void) {
  RESET_STATE();
  page_t* page  = &fake_page_pool[17];
  page->flags  |= PAGE_BUDDY;

  kfree(P2V(17 << PAGE_SHIFT));

  eq_num(buddy_free_called, 1, "kfree should call buddy_free for PAGE_BUDDY pages");
  eq_num(buddy_free_page, page, "kfree should pass correct page to buddy_free");
}

static void
kfree_calls_page_release_for_regular_page_test (void) {
  RESET_STATE();
  page_t* page = &fake_page_pool[99];
  page->flags  = 0;

  kfree(P2V(99 << PAGE_SHIFT));

  eq_num(page_release_called, 1, "kfree should call page_release for non-buddy pages");
  ok(page->flags & 0x1, "page should be marked as released");
}

// --- Main Test Driver ---

int
main (void) {
  plan(8);

  kmalloc_uses_buddy_if_small_enough_test();
  kmalloc_returns_0_if_too_large_test();
  kmalloc_falls_back_to_page_allocator_test();
  kmalloc_returns_0_if_no_pages_available_test();
  kfree_calls_buddy_free_for_buddy_page_test();
  kfree_calls_page_release_for_regular_page_test();

  done_testing();
}
