#ifndef MEM_PAGING_H
#define MEM_PAGING_H

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

typedef struct page page_t;

struct page {
  int     page_num;
  int     usage_count;
  int     flags;
  char   *data;
  page_t *prev_hash;
  page_t *next_hash;
  page_t *prev_free;
  page_t *next_free;
};

extern unsigned int *page_dir;

extern unsigned int page_pool_size;
extern page_t      *page_pool;

extern unsigned int page_hash_table_size;
extern page_t     **page_hash_table;

static const unsigned int blocksizes[] = {32, 64, 128, 256, 512, 1024, 2048, 4096};

/**
 * Initializes a temporary page directory so we can setup the higher-half kernel.
 *
 * @param magic The multiboot magic number, if extant.
 * @param mbi_ptr A pointer to the multiboot info structure, if extant.
 * @return unsigned int The address of the page directory.
 */
unsigned int mem_init_temporary(unsigned int magic, unsigned int mbi_ptr);

/**
 * Initializes the memory manager and permanent page directory. Also allocates kernel resources
 * beyond the higher-half static kernel memory boundary.
 */
void mem_init(void);

/**
 * Initializes the pages and populates the free-list.
 * @param num_pages
 */
void pages_init(unsigned int num_pages);

#endif /* MEM_PAGING_H */
