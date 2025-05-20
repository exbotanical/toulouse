#include "mem/alloc.h"

#include <stdlib.h>

#include "../stubs.h"
#include "lib/string.h"
#include "lib/types.h"
#include "libtap/libtap.h"
#include "mem/base.h"
#include "mem/page.h"

#define PAGE_POOL_SIZE 128

page_t  fake_page_pool[PAGE_POOL_SIZE];
page_t* page_pool                     = fake_page_pool;

unsigned int fake_addr                = 0xDEADBEEF;
unsigned int buddy_malloc_called_with = 0;
unsigned int buddy_malloc_return_val  = 0;
unsigned int page_release_called      = 0;
unsigned int buddy_free_called        = 0;
page_t*      buddy_free_page          = NULL;

unsigned int
buddy_malloc (size_t size) {
  buddy_malloc_called_with = size;
  return buddy_malloc_return_val;
}

void
buddy_free (page_t* page) {
  buddy_free_called++;
  buddy_free_page = page;
}

page_t*
page_get_free (void) {
  for (int i = 0; i < PAGE_POOL_SIZE; ++i) {
    if (!(fake_page_pool[i].flags & 0x1)) {
      fake_page_pool[i].page_num = i + 1;
      return &fake_page_pool[i];
    }
  }
  return NULL;
}

void
page_release (page_t* page) {
  page_release_called++;

  page->flags |= 0x1;
}

#define reset_mocks()                                \
  memset(fake_page_pool, 0, sizeof(fake_page_pool)); \
  buddy_malloc_called_with = 0;                      \
  buddy_malloc_return_val  = 0;                      \
  page_release_called      = 0;                      \
  buddy_free_called        = 0;                      \
  buddy_free_page          = NULL;

static void
kmalloc_uses_buddy_if_small_enough_test (void) {
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
  size_t too_large    = PAGE_SIZE + 1;

  unsigned int result = kmalloc(too_large);

  eq_num(result, 0, "kmalloc should return 0 if size > PAGE_SIZE");
}

static void
kmalloc_falls_back_to_page_allocator_test (void) {
  size_t large_size   = PAGE_SIZE;

  unsigned int result = kmalloc(large_size);

  eq_num(
    result,
    // TODO: Why not P2V working?
    KERNEL_PAGE_OFFSET + (1 << PAGE_SHIFT),
    "kmalloc should fallback to page allocator for large allocations"
  );
}

static void
kmalloc_returns_0_if_no_pages_available_test (void) {
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
  page_t* page       = &fake_page_pool[0];
  page->flags       |= PAGE_BUDDY;
  page->file_offset  = 99;
  free_page_list     = fake_page_pool;

  kfree(0xC0000001);

  eq_num(buddy_free_called, 1, "kfree should call buddy_free for PAGE_BUDDY pages");
}

static void
kfree_calls_page_release_for_regular_page_test (void) {
  page_t* page       = &fake_page_pool[0];
  page->flags       |= PAGE_RESERVED;
  page->file_offset  = 99;
  free_page_list     = fake_page_pool;

  kfree(0xC0000001);

  eq_num(page_release_called, 1, "kfree should call page_release for non-buddy pages");
}

int
main (void) {
  plan(7);

  reset_mocks();
  kmalloc_uses_buddy_if_small_enough_test();

  reset_mocks();
  kmalloc_returns_0_if_too_large_test();

  reset_mocks();
  kmalloc_falls_back_to_page_allocator_test();

  reset_mocks();
  kmalloc_returns_0_if_no_pages_available_test();

  // TODO: Test free actually operates on correct page
  reset_mocks();
  kfree_calls_buddy_free_for_buddy_page_test();

  reset_mocks();
  kfree_calls_page_release_for_regular_page_test();

  done_testing();
}
