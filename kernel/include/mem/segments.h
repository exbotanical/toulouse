#ifndef SEGMENTS_H
#define SEGMENTS_H
// Much of this code is copied from Jordi Sanfeliu's Fiwix - see licences.

#include "common/constants.h"
#include "mem/base.h"

/**
 * kernel code segment
 */
#define KERNEL_CS       0x08
/**
 * kernel data segment
 */
#define KERNEL_DS       0x10
/**
 * user code segment
 */
#define USER_CS         0x18
/**
 * user data segment
 */
#define USER_DS         0x20
/**
 * tss segment
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

#  include "common/compiler.h"

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

/**
 * Initializes the GDT for 32-bit protected mode.
 */
void gdt_init(void);

#endif  // ASM_SOURCE

#endif  // SEGMENTS_H
