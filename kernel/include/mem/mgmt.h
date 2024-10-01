#ifndef MEM_MGMT_H
#define MEM_MGMT_H

#define PAGE_SZ          4096
#define PAGE_SHIFT       0x0C
#define PAGE_MASK        ~(PAGE_SZ - 1)

// TODO: uppercase all macros
#define PAGE_ALIGN(addr) (((addr) + (PAGE_SZ - 1)) & PAGE_MASK)
#define GET_PGDIR(addr)  ((unsigned int)((addr) >> 22) & 0x3FF)

/* Page flags */
#define PAGE_PRESENT     0x001 /* Present */
#define PAGE_RW          0x002 /* Read/Write */
#define PAGE_USER        0x004 /* User */
#define PAGE_NOALLOC     0x200 /* No Page Allocated (OS managed) */

unsigned int tmp_paging_init(unsigned int magic, unsigned int mbi_ptr);

#endif /* MEM_MGMT_H */
