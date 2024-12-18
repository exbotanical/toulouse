#include "mem/paging.h"

#include "common/constants.h"
#include "config.h"
#include "debug/panic.h"
#include "init/bios.h"
#include "init/multiboot.h"
#include "kernel.h"
#include "kstat.h"
#include "lib/math.h"
#include "lib/string.h"
#include "mem/base.h"
#include "mem/paging.h"
#include "mem/segments.h"

static bl_head_t *freelist[BUDDY_MAX_LEVEL + 1];

unsigned int page_table_nbytes      = 0;
unsigned int page_hash_table_nbytes = 0;

unsigned int *page_dir;
page_t       *page_pool;      /* page pool */
page_t       *page_pool_head; /* page pool head */
page_t      **page_hash_table;

static inline void
activate_page_dir (void) {
  asm volatile("mov %0, %%cr3" ::"a"(page_dir));
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

static void
buddy_low_init (void) {
  k_memset(freelist, 0, sizeof(freelist));
}

unsigned int
tmp_paging_init (unsigned int magic, unsigned int mbi_ptr) {
  unsigned int num_pages = DEFAULT_NUM_PAGES;
  if (magic == MULTIBOOT_BOOTLOADER_MAGIC) {
    multiboot_info_t *mbi = (multiboot_info_t *)(mbi_ptr + KERNEL_PAGE_OFFSET);
    if (!(mbi->flags & MULTIBOOT_INFO_MEMORY)) {
      num_pages = DEFAULT_NUM_PAGES;
    } else {
      num_pages                       = (unsigned int)mbi->mem_upper + 1024;

      unsigned int max_possible_pages = (FOUR_GB - KERNEL_PAGE_OFFSET) / 1024;
      if (num_pages > max_possible_pages) {
        num_pages = max_possible_pages;
      }
    }
  }

  // Setup the temporary page directory
  unsigned int addr = KERNEL_PAGE_OFFSET + (num_pages * 1024) - num_pages;
  addr              = PAGE_ALIGN(addr);
  page_dir          = (unsigned int *)addr;
  k_memset(page_dir, 0, PAGE_SZ);

  // Setup the page table
  addr                     += PAGE_SZ;
  unsigned int *page_table  = (unsigned int *)addr;
  k_memset(page_table, 0, num_pages);

  unsigned int page_dir_idx;
  for (unsigned int idx = 0; idx < num_pages / sizeof(unsigned int); idx++) {
    page_table[idx] = (idx << PAGE_SHIFT) | PAGE_PRESENT | PAGE_RW;

    if (!(idx % 1024)) {
      page_dir_idx = idx / 1024;

      unsigned int page_value
        = (unsigned int)(addr + (PAGE_SZ * page_dir_idx) + GDT_BASE) | PAGE_PRESENT | PAGE_RW;
      page_dir[page_dir_idx]                                 = page_value;
      page_dir[page_dir_idx + GET_PGDIR(KERNEL_PAGE_OFFSET)] = page_value;
    }
  }

  return (unsigned int)page_dir - KERNEL_PAGE_OFFSET;
}

void
mem_init (void) {
  unsigned int physical_page_tables
    = (kstat.physical_pages / 1024) + ((kstat.physical_pages % 1024) ? 1 : 0);
  // unsigned int physical_memory = (kstat.physical_pages << PAGE_SHIFT);

  // align to the next page
  real_last_addr = PAGE_ALIGN(real_last_addr);

  // Page directory
  page_dir       = (unsigned int *)real_last_addr;
  k_memset(page_dir, 0, PAGE_SZ);
  real_last_addr           += PAGE_SZ;

  // Page tables
  unsigned int *page_table  = (unsigned int *)real_last_addr;
  k_memset((void *)page_table, 0, physical_page_tables * PAGE_SZ);
  real_last_addr += physical_page_tables * PAGE_SZ;

  // Initialize page dir + tables
  for (unsigned int n = 0; n < kstat.physical_pages; n++) {
    page_table[n] = (n << PAGE_SHIFT) | PAGE_PRESENT | PAGE_RW;
    if (!(n % 1024)) {
      page_dir[GET_PGDIR(KERNEL_PAGE_OFFSET) + (n / 1024)]
        = (unsigned int)&page_table[n] | PAGE_PRESENT | PAGE_RW;
    }
  }
  activate_page_dir();

  // We can now use virtual addresses
  page_dir               = (unsigned int *)P2V((unsigned int)page_dir);
  real_last_addr         = P2V(real_last_addr);

  // reserve memory space for RAMdisk drives
  //   last_ramdisk     = 0;
  //   if (kparm_ramdisksize > 0 || ramdisk_table[0].addr) {
  //     /*
  //      * If the 'initrd=' parameter was supplied, then the first
  //      * RAMdisk drive was already assigned to the initrd image.
  //      */
  //     if (ramdisk_table[0].addr) {
  //       ramdisk_table[0].addr += PAGE_OFFSET;
  //       last_ramdisk           = 1;
  //     }
  //     for (; last_ramdisk < ramdisk_minors; last_ramdisk++) {
  //       if (!bios_mmap_has_addr(V2P(real_last_addr) + (kparm_ramdisksize * 1024))) {
  //         kparm_ramdisksize  = 0;
  //         ramdisk_minors    -= RAMDISK_DRIVES;
  //         printk("WARNING: RAMdisk drive disabled (not enough physical memory).\n");
  //         break;
  //       }
  //       ramdisk_table[last_ramdisk].addr  = (char *)real_last_addr;
  //       ramdisk_table[last_ramdisk].size  = kparm_ramdisksize;
  //       real_last_addr                  += kparm_ramdisksize * 1024;
  //     }
  //   }

  // The last thing must be the page_table structure itself...
  int n                  = (kstat.physical_pages * PAGE_HASH_PER_10K) / 10000;
  n                      = max(n, 1); /* 1 page for the hash table as minimum */
  n                      = min(n, MAX_PAGES_HASH);

  page_hash_table_nbytes = n * PAGE_SZ;
  if (!bios_mmap_has_addr(V2P(real_last_addr) + page_hash_table_nbytes)) {
    k_panic("%s\n", "Not enough memory for page_hash_table");
  }
  page_hash_table    = (page_t **)real_last_addr;
  real_last_addr    += page_hash_table_nbytes;

  page_table_nbytes  = PAGE_ALIGN(kstat.physical_pages * sizeof(page_t));
  if (!bios_mmap_has_addr(V2P(real_last_addr) + page_table_nbytes)) {
    k_panic("%s\n", "Not enough memory for page_table");
  }
  page_pool       = (page_t *)real_last_addr;
  real_last_addr += page_table_nbytes;

  pages_init(kstat.physical_pages);
  buddy_low_init();
}

void
pages_init (unsigned int num_pages) {
  page_t *pg;

  k_memset(page_pool, 0, page_table_nbytes);
  k_memset(page_hash_table, 0, page_hash_table_nbytes);

  for (unsigned int n = 0; n < num_pages; n++) {
    pg                = &page_pool[n];
    pg->page          = n;

    unsigned int addr = n << PAGE_SHIFT;
    if (addr >= KERNEL_PHYSICAL_BASE && addr < V2P(real_last_addr)) {
      pg->flags = PAGE_RESERVED;
      kstat.kernel_reserved++;
      continue;
    }

    // reserve the kernel stack page
    if (addr == 0x0000F000) {
      pg->flags = PAGE_RESERVED;
      kstat.physical_reserved++;
      continue;
    }

    // Skip reserved memory addresses e.g. VGA adapter, BIOS, etc
    if (!bios_mmap_has_addr(addr)) {
      pg->flags = PAGE_RESERVED;
      kstat.physical_reserved++;
      continue;
    }

    pg->data = (char *)P2V(addr);
    free_list_insert(pg);
  }

  kstat.total_mem_pages = kstat.free_pages;
  kstat.min_free_pages  = (kstat.total_mem_pages * FREE_PAGES_RATIO) / 100;
}
