#include "mem/page.h"

#include "../stubs.h"
#include "kstat.h"
#include "libtap/libtap.h"

#define NUM_TEST_PAGES 32

static page_t  mock_page_pool[NUM_TEST_PAGES];
static page_t *mock_cache[NUM_TEST_PAGES];
extern kstat_t kstat;
extern page_t *free_page_list_head;
static void   *real_last_addr;

bool
bios_mmap_has_addr (unsigned int addr) {
  // Simulate only a few addresses being valid
  return (addr < 0x100000 || addr >= 0x200000);
}

#define P2V (p) p + 0xC0000000

#define V2P (v) v - 0xC0000000

void
wakeup (void *chan) {}

void
sleep (void *chan, int flags) {}

#define INTERRUPTS_OFF() \
  {}
#define INTERRUPTS_ON() \
  {}

void
kpanic (const char *fmt, ...) {
  diag("kpanic called");
  exit(1);
}

unsigned int
eflags_get (void) {
  return 1;
}

void
int_disable (void) {}

void
eflags_set (uint32_t eflags) {}

static void
page_init_reserves_and_initializes_free_list_test (void) {
  memset(mock_page_pool, 0, sizeof(mock_page_pool));
  memset(mock_cache, 0, sizeof(mock_cache));
  real_last_addr = (void *)0x200000;

  kstat          = (kstat_t){0};
  free_page_list = mock_page_pool;
  page_init(NUM_TEST_PAGES);

  unsigned int free_pages = 0, reserved_pages = 0;
  for (unsigned int i = 0; i < NUM_TEST_PAGES; i++) {
    if (mock_page_pool[i].flags == PAGE_RESERVED) {
      reserved_pages++;
    } else if (mock_page_pool[i].data) {
      free_pages++;
    }
  }

  ok(free_pages > 0, "Some pages were inserted into the free list");
  ok(reserved_pages > 0, "Some pages were reserved");

  eq_num(kstat.total_mem_pages, kstat.num_free_pages, "Total pages match free pages");

  ok(kstat.min_free_pages > 0, "Minimum free pages is computed");
}

static void
page_get_free_returns_page_test (void) {
  memset(mock_page_pool, 0, sizeof(mock_page_pool));
  free_page_list_head = &mock_page_pool[0];
  for (int i = 0; i < 3; i++) {
    mock_page_pool[i].next_free = &mock_page_pool[(i + 1) % 3];
    mock_page_pool[i].prev_free = &mock_page_pool[(i + 2) % 3];
  }

  kstat        = (kstat_t){.num_free_pages = 3, .min_free_pages = 0};

  page_t *page = page_get_free();
  ok(page != NULL, "Got a page");
  eq_num(kstat.num_free_pages, 2, "Free pages decremented");
  eq_num(page->usage_count, 1, "Usage count set to 1");
}

static void
page_release_returns_page_to_free_list_test (void) {
  memset(mock_page_pool, 0, sizeof(mock_page_pool));
  page_t *pg      = &mock_page_pool[1];
  pg->page_num    = 0;
  pg->usage_count = 1;

  pg->prev_free   = &mock_page_pool[0];
  kstat           = (kstat_t){.num_free_pages = 0, .total_mem_pages = NUM_TEST_PAGES};

  page_release(pg);

  eq_num(pg->usage_count, 0, "Usage count is 0 after release");
  eq_num(kstat.num_free_pages, 1, "Page is returned to free list");
  eq_num(free_page_list_head, pg, "Page is at head of free list");
}

static void
page_release_with_zero_use_count_does_nothing_test (void) {
  page_t *pg           = &mock_page_pool[1];
  pg->page_num         = 1;
  pg->usage_count      = 0;
  kstat.num_free_pages = 0;

  page_release(pg);
  eq_num(kstat.num_free_pages, 0, "No change if usage count was already 0");
}

static void
page_get_free_returns_null_if_empty_test (void) {
  kstat.num_free_pages  = 0;
  kstat.pages_reclaimed = 0;
  kstat.min_free_pages  = 1;

  page_t *pg            = page_get_free();
  eq_null(pg, "No page available, returns NULL");
}

int
main (void) {
  page_cache_size     = sizeof(mock_cache);
  free_page_list_size = sizeof(mock_page_pool);
  free_page_list      = mock_page_pool;
  page_cache          = mock_cache;

  plan(12);

  page_init_reserves_and_initializes_free_list_test();
  page_get_free_returns_page_test();
  page_release_returns_page_to_free_list_test();
  page_release_with_zero_use_count_does_nothing_test();
  page_get_free_returns_null_if_empty_test();

  done_testing();
}
