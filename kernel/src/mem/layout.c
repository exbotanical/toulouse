#include "mem/layout.h"

#include "debug/panic.h"
#include "drivers/dev/char/tmpcon.h"
#include "drivers/dev/char/video.h"
#include "init/bios.h"
#include "kconfig.h"
#include "kernel.h"
#include "lib/math.h"
#include "lib/string.h"
#include "mem/buddy.h"
#include "mem/page.h"
#include "mem/segments.h"
#include "proc/proc.h"

unsigned int *kpage_dir;

unsigned int free_page_list_size = 0;
page_t      *free_page_list;

unsigned int page_cache_size = 0;
page_t     **page_cache;

unsigned int proc_list_size = 0;

static inline unsigned int
mem_assign (unsigned int size, void **ptr, char *id) {
  unsigned int aligned_size = PAGE_ALIGN(size);
  if (!bios_mmap_has_addr(V2P(real_last_addr) + aligned_size)) {
    kpanic("Not enough memory for %s\n", id);
  }
  *ptr            = (void *)real_last_addr;
  real_last_addr += aligned_size;

  return aligned_size;
}

static inline void
mem_assign_cleared (unsigned int **ptr, unsigned int size) {
  *ptr = (unsigned int *)real_last_addr;
  kmemset(*ptr, 0, size);
  real_last_addr += size;
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
  kpage_dir         = (unsigned int *)addr;
  kmemset(kpage_dir, 0, PAGE_SIZE);

  addr                     += PAGE_SIZE;
  unsigned int *page_table  = (unsigned int *)addr;
  kmemset(page_table, 0, num_pages);

  for (unsigned int idx = 0; idx < num_pages / sizeof(unsigned int); idx++) {
    page_table[idx] = (idx << PAGE_SHIFT) | PAGE_PRESENT | PAGE_RW;

    if (!(idx % 1024)) {
      unsigned int kpage_dir_idx = idx / 1024;

      unsigned int page_value
        = (unsigned int)(addr + (PAGE_SIZE * kpage_dir_idx) + GDT_BASE) | PAGE_PRESENT | PAGE_RW;
      kpage_dir[kpage_dir_idx]                                 = page_value;
      kpage_dir[kpage_dir_idx + GET_PGDIR(KERNEL_PAGE_OFFSET)] = page_value;
    }
  }

  return (unsigned int)kpage_dir - KERNEL_PAGE_OFFSET;
}

// Identity maps kernel addresses. See https://stackoverflow.com/a/36872282
void
mem_init (void) {
  unsigned int physical_page_tables
    = (kstat.physical_pages / 1024) + ((kstat.physical_pages % 1024) ? 1 : 0);

  real_last_addr = PAGE_ALIGN(real_last_addr);
  mem_assign_cleared(&kpage_dir, PAGE_SIZE);

  unsigned int *page_table = (unsigned int *)real_last_addr;
  mem_assign_cleared(&page_table, physical_page_tables * PAGE_SIZE);

  for (unsigned int n = 0; n < kstat.physical_pages; n++) {
    page_table[n] = (n << PAGE_SHIFT) | PAGE_PRESENT | PAGE_RW;
    if (!(n % 1024)) {
      kpage_dir[GET_PGDIR(KERNEL_PAGE_OFFSET) + (n / 1024)]
        = (unsigned int)&page_table[n] | PAGE_PRESENT | PAGE_RW;
    }
  }

  page_activate_kpage_dir();
  // We can now use virtual addresses
  global_vga_con->buffer = (uint16_t *)P2V(VGA_ADDR);

  kpage_dir              = (unsigned int *)P2V((unsigned int)kpage_dir);
  real_last_addr         = P2V(real_last_addr);

  proc_list_size         = mem_assign(sizeof(proc_t) * NUM_PROCS, (void **)&proc_list, "proc_list");

  video_scrollback_history_buffer = (short int *)real_last_addr;
  real_last_addr
    += (video.columns * video.lines * VIDEO_MAX_SCROLLBACK_SCREENS * 2 * sizeof(short int));

  // The last thing must be the page table structure itself...
  unsigned int n  = (kstat.physical_pages * PAGE_HASH_PER_10K) / 10000;
  // 1 page for the hash table as minimum
  n               = max(n, 1);
  n               = min(n, MAX_PAGES_HASH);

  page_cache_size = mem_assign(n * PAGE_SIZE, (void **)&page_cache, "page_cache");

  free_page_list_size
    = mem_assign(kstat.physical_pages * sizeof(page_t), (void **)&free_page_list, "free_page_list");

  page_init(kstat.physical_pages);
  buddy_init();
}
