#ifndef MEM_PAGE_H
#define MEM_PAGE_H

#include "lib/types.h"

#define PAGE_SIZE           4096
#define PAGE_SHIFT          0x0C
#define PAGE_MASK           ~(PAGE_SIZE - 1)

/**
 * Aligns an address to the next page boundary.
 */
#define PAGE_ALIGN(addr)    (((addr) + (PAGE_SIZE - 1)) & PAGE_MASK)
/**
 * Computes the page directory address.
 */
#define GET_PGDIR(addr)     ((unsigned int)((addr) >> 22) & 0x3FF)
/**
 * Computes the page table address.
 */
#define GET_PGTBL(address)  ((unsigned int)((address) >> 12) & 0x3FF)

/* Page flags */

/**
 * Present
 */
#define PAGE_PRESENT        0x001
/**
 * Read/Write
 */
#define PAGE_RW             0x002
/**
 * User
 */
#define PAGE_USER           0x004
/**
 * No Page Allocated (OS managed)
 */
#define PAGE_NOALLOC        0x200

/**
 * Protection violation
 */
#define PAGE_FAULT_PROTVIOL 0x01
/**
 * Fault during write
 */
#define PAGE_FAULT_WRIT     0x02
/**
 * Fault while in user mode
 */
#define PAGE_FAULT_USRMOD   0x04

/**
 * Locked
 */
#define PAGE_LOCKED         0x001
/**
 * page belongs to buddy
 */
#define PAGE_BUDDY          0x010
/**
 * kernel, BIOS address, ...
 */
#define PAGE_RESERVED       0x100
/**
 * marked for Copy-On-Write
 */
#define PAGE_COW            0x200

/**
 * protection violation
 */
#define PFAULT_V            0x01
/**
 * during write
 */
#define PFAULT_W            0x02
/**
 * in user mode
 */
#define PFAULT_U            0x04

#define DEFAULT_NUM_PAGES   4096

#define NUM_PAGES           (free_page_list_size / sizeof(page_t))

typedef struct page page_t;

struct page {
  int            page_num;
  int            usage_count;
  int            flags;
  int            inode;
  unsigned int   file_offset;
  /**
   * Device on which the page resides
   */
  unsigned short dev;
  char          *data;
  page_t        *prev_hash;
  page_t        *next_hash;
  page_t        *prev_free;
  page_t        *next_free;
};

extern unsigned int *kpage_dir;

extern unsigned int free_page_list_size;
extern page_t      *free_page_list;

extern unsigned int page_cache_size;
extern page_t     **page_cache;

static inline void
page_activate_kpage_dir (void) {
  asm volatile("mov %0, %%cr3" ::"a"(kpage_dir));
}

static inline bool
page_is_valid (int page_num) {
  return (page_num >= 0 && page_num < (int)NUM_PAGES);
}

/**
 * Initializes the pages and populates the free-list.
 * @param num_pages
 */
void page_init(unsigned int num_pages);

/**
 * Grab a free page from the free list
 */
page_t *page_get_free(void);

/**
 * Release a page back into the free list
 */
void page_release(page_t *page);

#endif /* MEM_PAGE_H */
