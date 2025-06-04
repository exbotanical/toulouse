#ifndef MEM_SEGMENTS_H
#define MEM_SEGMENTS_H
// Much of this code is copied from Jordi Sanfeliu's Fiwix - see licences.

#include "lib/constants.h"
#include "mem/base.h"

/*
Note about TSS

In x86 protected mode, a Task State Segment (TSS) is a special data structure that holds information
about a task — stack pointers, segment selectors, and more. But critically:

* You don't load the TSS by giving its address. You give the CPU a selector — an index into the GDT.
* That selector points to a descriptor entry in the GDT which, in turn, points to the physical
memory location of the TSS.
*/

/**
 * kernel code segment selector
 */
#define KERNEL_CS       0x08
/**
 * kernel data segment selector
 */
#define KERNEL_DS       0x10
/**
 * user code segment selector
 */
#define USER_CS         0x18
/**
 * user data segment selector
 */
#define USER_DS         0x20
/**
 * tss segment selector
 */
#define TSS             0x28

/* Low flags of segment descriptors */

/**
 * code r/x
 */
#define SD_CODE         0x0A
/**
 * data r/w
 */
#define SD_DATA         0x02

/**
 * 32-bit interrupt gate
 */
#define SD_32INTRGATE   0x0E
/**
 * 32-bit trap (exception) gate
 */
#define SD_32TRAPGATE   0x0F

/**
 * 0 = system / 1 = code/data
 */
#define SD_CD           0x10
/**
 * priority level 0 (kernel)
 */
#define SD_DPL0         0x00
/**
 * priority level 3 (user)
 */
#define SD_DPL3         0x60
/**
 * segment present / gate valid
 */
#define SD_PRESENT      0x80

/**
 * 32-bit code and data segments
 */
#define SD_OPSIZE32     0x04
/**
 * page granularity (4KB)
 */
#define SD_PAGE4KB      0x08

/**
 * tss present + not busy
 */
#define SD_TSS_PRESENT  0x89

#define NUM_GDT_ENTRIES 6
#define GDT_BASE        (FOUR_GB - (KERNEL_PAGE_OFFSET - 1))

#ifndef ASM_SOURCE

#  include "lib/compiler.h"

typedef struct {
  /**
   * segment limit 0-15 bits
   */
  unsigned low_limit : 16;
  /**
   * base address 0-23 bits
   */
  unsigned low_base  : 24;
  /**
   * flags (P, DPL, S and TYPE)
   */
  unsigned low_flags : 8;
  /**
   * segment limit 16-19 bits
   */
  unsigned hi_limit  : 4;
  /**
   * flags (G, DB, 0 and AVL)
   */
  unsigned hi_flags  : 4;
  /**
   * base address 24-31 bits
   */
  unsigned hi_base   : 8;
} packed seg_desc_t;

typedef struct {
  /**
   * offset 0-15 bits
   */
  unsigned low_offset : 16;
  /**
   * segment selector
   */
  unsigned selector   : 16;
  /**
   * flags (P, DPL, TYPE, 0 and NULL)
   */
  unsigned flags      : 16;
  /**
   * offset 16-31 bits
   */
  unsigned hi_offset  : 16;
} packed gate_desc_t;

typedef struct {
  uint16_t limit;
  uint32_t base_addr;
} packed descr_t;

extern seg_desc_t gdt[NUM_GDT_ENTRIES];

/**
 * Initializes the GDT for 32-bit protected mode.
 */
void gdt_init(void);

#endif  // ASM_SOURCE

#endif  // MEM_SEGMENTS_H
