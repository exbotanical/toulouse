#ifndef SEGMENTS_H
#define SEGMENTS_H
// Much of this code is copied from Jordi Sanfeliu's Fiwix

#include "mem/base.h"

#define KERNEL_CS       0x08 /* kernel code segment */
#define KERNEL_DS       0x10 /* kernel data segment */
#define USER_CS         0x18 /* user code segment */
#define USER_DS         0x20 /* user data segment */
#define TSS             0x28 /* TSS segment */

/* Low flags of segment descriptors */
#define SD_CODE         0x0A /* CODE Exec/Read */
#define SD_DATA         0x02 /* DATA Read/Write */

#define SD_32INTRGATE   0x0E /* 32-bit Interrupt Gate (0D110) */
#define SD_32TRAPGATE   0x0F /* 32-bit Trap Gate (0D111) */

#define SD_CD           0x10 /* 0 = system / 1 = code/data */
#define SD_DPL0         0x00 /* priority level 0 (kernel) */
#define SD_DPL3         0x60 /* priority level 3 (user) */
#define SD_PRESENT      0x80 /* segment present or valid */

/* High flags segment descriptors */
#define SD_OPSIZE32     0x04 /* 32-bit code and data segments */
#define SD_PAGE4KB      0x08 /* page granularity (4KB) */

/* Low flags of the TSS descriptors */
#define SD_TSS_PRESENT  0x89 /* TSS present and not busy flag */

#define NUM_GDT_ENTRIES 6
#define GDT_BASE        (0xFFFFFFFF - (KERNEL_PAGE_OFFSET - 1))

#ifndef ASM_SOURCE

#  include "common/compiler.h"

typedef struct {
  unsigned lo_limit : 16; /* segment limit 0-15 bits */
  unsigned lo_base  : 24; /* base address 0-23 bits */
  unsigned lo_flags : 8;  /* flags (P, DPL, S and TYPE) */
  unsigned hi_limit : 4;  /* segment limit 16-19 bits */
  unsigned hi_flags : 4;  /* flags (G, DB, 0 and AVL) */
  unsigned hi_base  : 8;  /* base address 24-31 bits */
} packed seg_desc_t;

typedef struct {
  uint16_t limit;
  uint32_t base_addr;
} packed descr_t;

void gdt_init(void);

#endif /* ASM_SOURCE */

#endif /* SEGMENTS_H */
