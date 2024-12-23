#include "mem/paging.h"

#include "common/constants.h"
#include "config.h"
#include "debug/panic.h"
#include "drivers/console/vga.h"
#include "drivers/dev/char/console.h"
#include "init/bios.h"
#include "init/multiboot.h"
#include "kernel.h"
#include "kstat.h"
#include "lib/math.h"
#include "lib/string.h"
#include "mem/base.h"
#include "mem/buddy.h"
#include "mem/paging.h"
#include "mem/segments.h"

unsigned int page_pool_size       = 0;
unsigned int page_hash_table_size = 0;

unsigned int *page_dir;
page_t       *page_pool;      /* page pool */
page_t       *page_pool_head; /* page pool head */
page_t      **page_hash_table;

static inline void
activate_page_dir (void) {
  asm volatile("mov %0, %%cr3" ::"a"(page_dir));
}

static inline void
flag_kreserved (page_t *page) {
  page->flags = PAGE_RESERVED;
  kstat.kernel_reserved++;
}

static inline void
flag_preserved (page_t *page) {
  page->flags = PAGE_RESERVED;
  kstat.physical_reserved++;
}

static void
free_list_insert (page_t *pg) {
  if (!page_pool_head) {
    pg->prev_free = pg->next_free = pg;
    page_pool_head                = pg;
  } else {
    pg->next_free                        = page_pool_head;
    pg->prev_free                        = page_pool_head->prev_free;
    page_pool_head->prev_free->next_free = pg;
    page_pool_head->prev_free            = pg;
  }

  kstat.free_pages++;
}

unsigned int
mem_init_temporary (unsigned int magic, unsigned int mbi_ptr) {
  unsigned int num_pages = DEFAULT_NUM_PAGES;
  if (magic == MULTIBOOT_BOOTLOADER_MAGIC) {
    multiboot_info_t *mbi = (multiboot_info_t *)(mbi_ptr + KERNEL_PAGE_OFFSET);
    if ((mbi->flags & MULTIBOOT_INFO_MEMORY)) {
      unsigned int max_possible_pages = (FOUR_GB - KERNEL_PAGE_OFFSET) / 1024;
      num_pages = min((unsigned int)mbi->mem_upper + 1024, max_possible_pages);
    }
  }

  unsigned int addr = PAGE_ALIGN(KERNEL_PAGE_OFFSET + (num_pages * 1024) - num_pages);
  page_dir          = (unsigned int *)addr;
  k_memset(page_dir, 0, PAGE_SIZE);

  addr                     += PAGE_SIZE;
  unsigned int *page_table  = (unsigned int *)addr;
  k_memset(page_table, 0, num_pages);

  for (unsigned int idx = 0; idx < num_pages / sizeof(unsigned int); idx++) {
    page_table[idx] = (idx << PAGE_SHIFT) | PAGE_PRESENT | PAGE_RW;

    if (!(idx % 1024)) {
      unsigned int page_dir_idx = idx / 1024;

      unsigned int page_value
        = (unsigned int)(addr + (PAGE_SIZE * page_dir_idx) + GDT_BASE) | PAGE_PRESENT | PAGE_RW;
      page_dir[page_dir_idx]                                 = page_value;
      page_dir[page_dir_idx + GET_PGDIR(KERNEL_PAGE_OFFSET)] = page_value;
    }
  }

  return (unsigned int)page_dir - KERNEL_PAGE_OFFSET;
}

// Identity maps kernel addresses. See https://stackoverflow.com/a/36872282
void
mem_init (void) {
  unsigned int physical_page_tables
    = (kstat.physical_pages / 1024) + ((kstat.physical_pages % 1024) ? 1 : 0);

  real_last_addr = PAGE_ALIGN(real_last_addr);

  page_dir       = (unsigned int *)real_last_addr;
  k_memset(page_dir, 0, PAGE_SIZE);
  real_last_addr           += PAGE_SIZE;

  unsigned int *page_table  = (unsigned int *)real_last_addr;
  k_memset((void *)page_table, 0, physical_page_tables * PAGE_SIZE);
  real_last_addr += physical_page_tables * PAGE_SIZE;

  for (unsigned int n = 0; n < kstat.physical_pages; n++) {
    page_table[n] = (n << PAGE_SHIFT) | PAGE_PRESENT | PAGE_RW;
    if (!(n % 1024)) {
      page_dir[GET_PGDIR(KERNEL_PAGE_OFFSET) + (n / 1024)]
        = (unsigned int)&page_table[n] | PAGE_PRESENT | PAGE_RW;
    }
  }

  activate_page_dir();

  global_vga_con->buffer = (uint16_t *)P2V(VGA_ADDR);

  // We can now use virtual addresses
  page_dir               = (unsigned int *)P2V((unsigned int)page_dir);
  real_last_addr         = P2V(real_last_addr);
  // real_last_addr         += (video.columns * video.rows * 2 * sizeof(uint16_t));

  // The last thing must be the page table structure itself...
  unsigned int n         = (kstat.physical_pages * PAGE_HASH_PER_10K) / 10000;
  // 1 page for the hash table as minimum
  n                      = max(n, 1);
  n                      = min(n, MAX_PAGES_HASH);

  page_hash_table_size   = n * PAGE_SIZE;
  if (!bios_mmap_has_addr(V2P(real_last_addr) + page_hash_table_size)) {
    k_panic("%s\n", "Not enough memory for page_hash_table");
  }
  page_hash_table  = (page_t **)real_last_addr;
  real_last_addr  += page_hash_table_size;

  page_pool_size   = PAGE_ALIGN(kstat.physical_pages * sizeof(page_t));
  if (!bios_mmap_has_addr(V2P(real_last_addr) + page_pool_size)) {
    k_panic("%s\n", "Not enough memory for page_table");
  }
  page_pool       = (page_t *)real_last_addr;
  real_last_addr += page_pool_size;

  pages_init(kstat.physical_pages);
  kbuddy_init();
}

void
pages_init (unsigned int num_pages) {
  k_memset(page_pool, 0, page_pool_size);
  k_memset(page_hash_table, 0, page_hash_table_size);

  for (unsigned int n = 0; n < num_pages; n++) {
    page_t *page      = &page_pool[n];
    page->page_num    = n;

    // Flag the kernel pages as reserved
    unsigned int addr = n << PAGE_SHIFT;
    if (addr >= KERNEL_PHYSICAL_BASE && addr < V2P(real_last_addr)) {
      flag_kreserved(page);
      continue;
    }

    // Reserve a page for the kernel stack
    if (addr == 0x0000F000) {
      flag_preserved(page);
      continue;
    }

    // Flag special memory addresses e.g. VGA, BIOS, etc as reserved
    if (!bios_mmap_has_addr(addr)) {
      flag_preserved(page);
      continue;
    }

    // Everything else can be inserted into the free page list
    page->data = (char *)P2V(addr);
    free_list_insert(page);
  }

  kstat.total_mem_pages = kstat.free_pages;
  kstat.min_free_pages  = (kstat.total_mem_pages * FREE_PAGES_RATIO) / 100;
}
