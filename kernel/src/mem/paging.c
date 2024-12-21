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
#include "mem/allocator.h"
#include "mem/base.h"
#include "mem/buddy.h"
#include "mem/paging.h"
#include "mem/segments.h"

unsigned int page_table_bytes      = 0;
unsigned int page_hash_table_bytes = 0;

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

unsigned int
map_kaddr (
  unsigned int *lpage_dir,
  unsigned int  from,
  unsigned int  to,
  unsigned int  addr,
  int           flags
) {
  unsigned int  n;
  unsigned int  paddr;
  unsigned int *pgtbl;
  unsigned int  pde, pte;

  paddr = addr;
  for (n = from; n < to; n += PAGE_SZ) {
    pde = GET_PGDIR(n);
    pte = GET_PGTBL(n);
    if (!(lpage_dir[pde] & ~PAGE_MASK)) {
      if (!addr) {
        paddr = k_malloc(PAGE_SZ);
        if (!paddr) {
          vgaprintf("%s(): no memory\n", __func__);
          return 0;
        }
        paddr = V2P(paddr);
      }
      lpage_dir[pde] = paddr | flags;
      k_memset((void *)(paddr + KERNEL_PAGE_OFFSET), 0, PAGE_SZ);
      paddr += PAGE_SZ;
    }
    pgtbl      = (unsigned int *)((lpage_dir[pde] & PAGE_MASK) + KERNEL_PAGE_OFFSET);
    pgtbl[pte] = n | flags;
  }

  return paddr;
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

// Identity maps kernel addresses. See https://stackoverflow.com/a/36872282
void
mem_init (void) {
  unsigned int physical_page_tables
    = (kstat.physical_pages / 1024) + ((kstat.physical_pages % 1024) ? 1 : 0);

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

  global_vga_con->buffer = (uint16_t *)P2V(VGA_ADDR);

  // We can now use virtual addresses
  page_dir               = (unsigned int *)P2V((unsigned int)page_dir);
  real_last_addr         = P2V(real_last_addr);
  // real_last_addr         += (video.columns * video.rows * 2 * sizeof(uint16_t));

  // The last thing must be the page_table structure itself...
  int n                  = (kstat.physical_pages * PAGE_HASH_PER_10K) / 10000;
  n                      = max(n, 1); /* 1 page for the hash table as minimum */
  n                      = min(n, MAX_PAGES_HASH);

  page_hash_table_bytes  = n * PAGE_SZ;
  if (!bios_mmap_has_addr(V2P(real_last_addr) + page_hash_table_bytes)) {
    k_panic("%s\n", "Not enough memory for page_hash_table");
  }
  page_hash_table   = (page_t **)real_last_addr;
  real_last_addr   += page_hash_table_bytes;

  page_table_bytes  = PAGE_ALIGN(kstat.physical_pages * sizeof(page_t));
  if (!bios_mmap_has_addr(V2P(real_last_addr) + page_table_bytes)) {
    k_panic("%s\n", "Not enough memory for page_table");
  }
  page_pool       = (page_t *)real_last_addr;
  real_last_addr += page_table_bytes;

  pages_init(kstat.physical_pages);
  buddy_low_init();
}

void
pages_init (unsigned int num_pages) {
  k_memset(page_pool, 0, page_table_bytes);
  k_memset(page_hash_table, 0, page_hash_table_bytes);

  for (unsigned int n = 0; n < num_pages; n++) {
    page_t *pg        = &page_pool[n];
    pg->page          = n;

    // Flag the kernel pages as reserved
    unsigned int addr = n << PAGE_SHIFT;
    if (addr >= KERNEL_PHYSICAL_BASE && addr < V2P(real_last_addr)) {
      pg->flags = PAGE_RESERVED;
      kstat.kernel_reserved++;
      continue;
    }

    // Reserve a page for the kernel stack
    if (addr == 0x0000F000) {
      pg->flags = PAGE_RESERVED;
      kstat.physical_reserved++;
      continue;
    }

    // Flag special memory addresses e.g. VGA, BIOS, etc as reserved
    if (!bios_mmap_has_addr(addr)) {
      pg->flags = PAGE_RESERVED;
      kstat.physical_reserved++;
      continue;
    }

    // Everything else can be inserted into the free page list
    pg->data = (char *)P2V(addr);
    free_list_insert(pg);
  }

  kstat.total_mem_pages = kstat.free_pages;
  kstat.min_free_pages  = (kstat.total_mem_pages * FREE_PAGES_RATIO) / 100;
}
