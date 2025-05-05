#include "mem/paging.h"

#include "common/constants.h"
#include "debug/panic.h"
#include "drivers/console/tmpcon.h"
#include "drivers/dev/video.h"
#include "init/bios.h"
#include "init/multiboot.h"
#include "kconfig.h"
#include "kernel.h"
#include "kstat.h"
#include "lib/math.h"
#include "lib/string.h"
#include "mem/base.h"
#include "mem/buddy.h"
#include "mem/paging.h"
#include "mem/segments.h"
#include "proc/process.h"

unsigned int *page_dir;

unsigned int page_pool_size = 0;
page_t      *page_pool;
page_t      *page_pool_head;

unsigned int page_hash_table_size = 0;
page_t     **page_hash_table;

unsigned int proc_table_size = 0;

static inline unsigned int
mem_assign (unsigned int size, void **ptr, char *id) {
  unsigned int aligned_size = PAGE_ALIGN(size);
  if (!bios_mmap_has_addr(V2P(real_last_addr) + aligned_size)) {
    kpanic("Not enough memory for %s\n", id);
  }
  *ptr            = (process_t *)real_last_addr;
  real_last_addr += aligned_size;

  return aligned_size;
}

static inline void
mem_assign_cleared (unsigned int **ptr, unsigned int size) {
  *ptr = (unsigned int *)real_last_addr;
  kmemset(*ptr, 0, size);
  real_last_addr += size;
}

static inline void
mem_activate_page_dir (void) {
  asm volatile("mov %0, %%cr3" ::"a"(page_dir));
}

static inline void
mem_flag_kreserved (page_t *page) {
  page->flags = PAGE_RESERVED;
  kstat.kernel_reserved++;
}

static inline void
mem_flag_preserved (page_t *page) {
  page->flags = PAGE_RESERVED;
  kstat.physical_reserved++;
}

static void
mem_free_list_insert (page_t *pg) {
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
  kmemset(page_dir, 0, PAGE_SIZE);

  addr                     += PAGE_SIZE;
  unsigned int *page_table  = (unsigned int *)addr;
  kmemset(page_table, 0, num_pages);

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
  mem_assign_cleared(&page_dir, PAGE_SIZE);

  unsigned int *page_table = (unsigned int *)real_last_addr;
  mem_assign_cleared(&page_table, physical_page_tables * PAGE_SIZE);

  for (unsigned int n = 0; n < kstat.physical_pages; n++) {
    page_table[n] = (n << PAGE_SHIFT) | PAGE_PRESENT | PAGE_RW;
    if (!(n % 1024)) {
      page_dir[GET_PGDIR(KERNEL_PAGE_OFFSET) + (n / 1024)]
        = (unsigned int)&page_table[n] | PAGE_PRESENT | PAGE_RW;
    }
  }

  mem_activate_page_dir();
  // We can now use virtual addresses
  global_vga_con->buffer = (uint16_t *)P2V(VGA_ADDR);

  page_dir               = (unsigned int *)P2V((unsigned int)page_dir);
  real_last_addr         = P2V(real_last_addr);

  proc_table_size
    = mem_assign(sizeof(process_t) * NUM_PROCESSES, (void **)&proc_table, "proc_table");

  video_scrollback_history_buffer = (short int *)real_last_addr;
  real_last_addr
    += (video.columns * video.lines * VIDEO_MAX_SCROLLBACK_SCREENS * 2 * sizeof(short int));

  // The last thing must be the page table structure itself...
  unsigned int n       = (kstat.physical_pages * PAGE_HASH_PER_10K) / 10000;
  // 1 page for the hash table as minimum
  n                    = max(n, 1);
  n                    = min(n, MAX_PAGES_HASH);

  page_hash_table_size = mem_assign(n * PAGE_SIZE, (void **)&page_hash_table, "page_hash_table");

  page_pool_size
    = mem_assign(kstat.physical_pages * sizeof(page_t), (void **)&page_pool, "page_pool");

  pages_init(kstat.physical_pages);
  kbuddy_init();
}

void
pages_init (unsigned int num_pages) {
  kmemset(page_pool, 0, page_pool_size);
  kmemset(page_hash_table, 0, page_hash_table_size);

  for (unsigned int n = 0; n < num_pages; n++) {
    page_t *page      = &page_pool[n];
    page->page_num    = n;

    // Flag the kernel pages as reserved
    unsigned int addr = n << PAGE_SHIFT;
    if (addr >= KERNEL_PHYSICAL_BASE && addr < V2P(real_last_addr)) {
      mem_flag_kreserved(page);
      continue;
    }

    // Reserve a page for the kernel stack
    if (addr == 0x0000F000) {
      mem_flag_preserved(page);
      continue;
    }

    // Flag special memory addresses e.g. VGA, BIOS, etc as reserved
    if (!bios_mmap_has_addr(addr)) {
      mem_flag_preserved(page);
      continue;
    }

    // Everything else can be inserted into the free page list
    page->data = (char *)P2V(addr);
    mem_free_list_insert(page);
  }

  kstat.total_mem_pages = kstat.free_pages;
  kstat.min_free_pages  = (kstat.total_mem_pages * FREE_PAGES_RATIO) / 100;
}
