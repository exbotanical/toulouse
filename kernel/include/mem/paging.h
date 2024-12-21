#ifndef PAGING_H
#define PAGING_H

#define PAGE_SZ             4096
#define PAGE_SHIFT          0x0C
#define PAGE_MASK           ~(PAGE_SZ - 1)

// TODO: uppercase all macros
#define PAGE_ALIGN(addr)    (((addr) + (PAGE_SZ - 1)) & PAGE_MASK)
#define GET_PGDIR(addr)     ((unsigned int)((addr) >> 22) & 0x3FF)
#define GET_PGTBL(address)  ((unsigned int)((address) >> 12) & 0x3FF)
#define PT_ENTRIES          (PAGE_SZ / sizeof(unsigned int))
#define PD_ENTRIES          (PAGE_SZ / sizeof(unsigned int))

/* Page flags */
#define PAGE_PRESENT        0x001 /* Present */
#define PAGE_RW             0x002 /* Read/Write */
#define PAGE_USER           0x004 /* User */
#define PAGE_NOALLOC        0x200 /* No Page Allocated (OS managed) */

#define PAGE_FAULT_PROTVIOL 0x01  /* Protection violation */
#define PAGE_FAULT_WRIT     0x02  /* Fault during write */
#define PAGE_FAULT_USRMOD   0x04  /* Fault while in user mode */

#define PAGE_LOCKED         0x001
#define PAGE_BUDDYLOW       0x010 /* page belongs to buddy_low */
#define PAGE_RESERVED       0x100 /* kernel, BIOS address, ... */
#define PAGE_COW            0x200 /* marked for Copy-On-Write */

#define PFAULT_V            0x01  /* protection violation */
#define PFAULT_W            0x02  /* during write */
#define PFAULT_U            0x04  /* in user mode */

#define DEFAULT_NUM_PAGES   4096

typedef struct page page_t;

struct page {
  int     page;   // Page number
  int     count;  // Usage counter
  int     flags;
  char   *data;   // Page contents
  page_t *prev_hash;
  page_t *next_hash;
  page_t *prev_free;
  page_t *next_free;
};

extern unsigned int page_table_bytes;
extern unsigned int page_hash_table_bytes;

extern unsigned int *page_dir;

extern page_t  *page_pool;
extern page_t **page_hash_table;

static const unsigned int blocksizes[] = {32, 64, 128, 256, 512, 1024, 2048, 4096};

unsigned int tmp_paging_init(unsigned int magic, unsigned int mbi_ptr);
void         mem_init(void);
void         pages_init(unsigned int num_pages);
unsigned int map_kaddr(
  unsigned int *lpage_dir,
  unsigned int  from,
  unsigned int  to,
  unsigned int  addr,
  int           flags
);

#endif /* PAGING_H */
