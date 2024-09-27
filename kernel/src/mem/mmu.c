#include "mem/mmu.h"

#include "arch/interrupt.h"
#include "drivers/console/vga.h"
#include "lib/string.h"
#include "mem/manager.h"
#include "sync/spinlock.h"

static const uint32_t base_offset = KERNEL_VIRTUAL_BASE / PAGE_SZ / NUM_ENTRIES;

/**
 * The page which will be mapped to kernel address space on the next allocation.
 */
static uint32_t next_page         = 0;

static page_dir_t page_directory aligned(PAGE_SZ);

static page_table_t page_tables[KERNEL_NUM_TABLES] aligned(PAGE_SZ);

static inline phys_addr_t
v_to_phys (void* kaddr) {
  return ((uint32_t)kaddr) - KERNEL_VIRTUAL_BASE;
}

/**
 * Invalidates the TLB for a given page.
 *
 * @param addr
 */
static inline void
invalidate_page (uint32_t addr) {
  asm volatile("invlpg (%0)" ::"r"(addr) : "memory");
}

static inline void
loadcr3 (uint32_t phys) {
  asm volatile("mov %0, %%cr3" ::"a"(phys));
}

/**
 * Retrieves the page directory index for a given physical address.
 *
 * @param addr
 * @return uint32_t
 */
static inline uint32_t
get_page_dir (phys_addr_t addr) {
  // Keep the higher 20 bits (i.e. mask off the lower 12 - the page offset)
  uint32_t aligned_addr = addr & 0xFFFFF000;
  return (aligned_addr / PAGE_SZ) / NUM_ENTRIES;
}

static inline uint32_t
get_page_table (phys_addr_t addr) {
  // Keep the higher 20 bits (i.e. mask off the lower 12 - the page offset)
  uint32_t aligned_addr = addr & 0xFFFFF000;
  return (aligned_addr / PAGE_SZ) % NUM_ENTRIES;
}

/**
 * Returns the page offset of a given address.
 *
 * @param addr
 * @return uint32_t
 */
static inline uint32_t
get_page_offset (uint32_t addr) {
  // Uses 4095 (highest possible page no) to retain the upper 12 bits.1
  return addr & 0xFFF;
}

/**
 * Returns the next virtual address to map.
 *
 * @param phys_addr
 * @return uint32_t
 */
static inline uint32_t
get_next_virtual (uint32_t phys_addr) {
  return (next_page * PAGE_SZ) + KERNEL_VIRTUAL_BASE + get_page_offset(phys_addr);
}

/**
 * Sets the entry in the given page table.
 *
 * @param t
 * @param idx
 * @param phys_addr
 * @param flags
 */
static inline void
tableent_set (page_table_t* t, uint32_t idx, phys_addr_t phys_addr, uint32_t flags) {
  t->e[idx].phys_addr = phys_addr >> 12;
  t->e[idx].flags     = flags & 0xFFF;
}

static inline void
dirent_set (page_dir_t* d, uint32_t idx, phys_addr_t phys, uint32_t flags) {
  d->e[idx].phys_addr = phys >> 12;
  d->e[idx].flags     = flags & 0xFFF;
}

static inline void
dirent_clear (page_dir_t* t, uint32_t idx) {
  dirent_set(t, idx, 0, 0);
}

static void
build_page_dir (page_dir_t* dir) {
  for (uint32_t i = 0; i < NUM_ENTRIES - KERNEL_NUM_TABLES; i++) {
    dirent_clear(dir, i);
  }

  for (uint32_t i = 0; i < KERNEL_NUM_TABLES; i++) {
    phys_addr_t phys_addr = v_to_phys(&page_tables[i]);
    dirent_set(dir, i + base_offset, phys_addr, MMU_FLAG_PRESENT | MMU_FLAG_WRITABLE);
  }
}

/**
 * Maps a page at the given physical address, adding the entry to the current page table in the
 * current page directory.
 *
 * @param phys_addr
 */
static void
setup_page (phys_addr_t phys_addr) {
  uint32_t next_page_boundary = get_next_virtual(0);
  next_page++;

  // TODO: get_page_dir(KERNEL_VIRTUAL_BASE) const
  uint32_t idx = get_page_dir(next_page_boundary) - get_page_dir(KERNEL_VIRTUAL_BASE);
  tableent_set(
    &page_tables[idx],
    get_page_table(next_page_boundary),
    phys_addr,
    MMU_FLAG_WRITABLE | MMU_FLAG_PRESENT
  );

  // Invalidate any old/pre-existing TLB entry
  invalidate_page(next_page_boundary);
}

/**
 * Maps a single page starting at `phys_addr`.
 *
 * @param phys_addr
 * @return uint32_t
 */
uint32_t
map_page (phys_addr_t phys_addr) {
  return map_pages(phys_addr, 1);
}

/**
 * Maps `pages` pages starting at `phys_addr`.
 *
 * @param phys_addr
 * @param pages
 * @return uint32_t The virtual address of the first page mapped.
 */
uint32_t
map_pages (phys_addr_t phys_addr, uint32_t pages) {
  uint32_t   flags;
  spinlock_t lock;
  spinlock_init(&lock);

  int_save_disable(&flags);
  spinlock_lock(&lock);

  uint32_t first_page_start = get_next_virtual(phys_addr);
  for (uint32_t i = 0; i < pages; i++) {
    setup_page(phys_addr + (PAGE_SZ * i));
  }

  spinlock_unlock(&lock);
  int_store(flags);

  return first_page_start;
}

uint32_t
mmu_init (phys_addr_t kernel_end, phys_addr_t heap_start) {
  next_page = 0;
  // Map 0xC0000000->0x00000000, 0xC0001000->0x00001000, ...
  // We're essentially mapping over the entire kernel image, then the page table list immediately
  // thereafter.
  map_pages(0, div_up(kernel_end, PAGE_SZ));               // 0 - 2162753 (529 pages)
  uint32_t pages = map_pages(heap_start, NUM_HEAP_PAGES);  // 4625 - 5120

  build_page_dir(&page_directory);
  loadcr3(v_to_phys(&page_directory));

  // Map physical addresses such as the VGA so they continue to work
  vga_early_remap(global_vga_con);

  return pages;
}
