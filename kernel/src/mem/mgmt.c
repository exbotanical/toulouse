#include "mem/mgmt.h"

#include "init/multiboot.h"
#include "lib/string.h"
#include "mem/base.h"
#include "mem/segments.h"

#define DEFAULT_NUM_PAGES 4096

unsigned int *pg_dir;
unsigned int *pg_tbl;

unsigned int
tmp_paging_init (unsigned int magic, unsigned int mbi_ptr) {
  unsigned int num_pgs = DEFAULT_NUM_PAGES;
  if (magic == MULTIBOOT_BOOTLOADER_MAGIC) {
    multiboot_info_t *mbi = (multiboot_info_t *)(mbi_ptr + KERNEL_PAGE_OFFSET);
    if (!(mbi->flags & MULTIBOOT_INFO_MEMORY)) {
      num_pgs = DEFAULT_NUM_PAGES;
    } else {
      num_pgs = (unsigned int)mbi->mem_upper + 1024;

      if (num_pgs > ((0xFFFFFFFF - KERNEL_PAGE_OFFSET) / 1024)) {
        num_pgs = (0xFFFFFFFF - KERNEL_PAGE_OFFSET) / 1024;
      }
    }
  }

  // Setup the temporary pg directory
  unsigned int addr = KERNEL_PAGE_OFFSET + (num_pgs * 1024) - num_pgs;
  addr              = PAGE_ALIGN(addr);
  pg_dir            = (unsigned int *)addr;
  k_memset(pg_dir, 0, PAGE_SZ);

  // Setup the page table
  addr   += PAGE_SZ;
  pg_tbl  = (unsigned int *)addr;
  k_memset(pg_tbl, 0, num_pgs);

  unsigned int pg_dir_idx;
  for (unsigned int idx = 0; idx < num_pgs / sizeof(unsigned int); idx++) {
    pg_tbl[idx] = (idx << PAGE_SHIFT) | PAGE_PRESENT | PAGE_RW;

    if (!(idx % 1024)) {
      pg_dir_idx = idx / 1024;
      pg_dir[pg_dir_idx]
        = (unsigned int)(addr + (PAGE_SZ * pg_dir_idx) + GDT_BASE) | PAGE_PRESENT | PAGE_RW;
      pg_dir[pg_dir_idx + GET_PGDIR(KERNEL_PAGE_OFFSET)]
        = (unsigned int)(addr + (PAGE_SZ * pg_dir_idx) + GDT_BASE) | PAGE_PRESENT | PAGE_RW;
    }
  }

  return (unsigned int)pg_dir - KERNEL_PAGE_OFFSET;
}
