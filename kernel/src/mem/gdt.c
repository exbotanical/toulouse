#include "arch/x86.h"
#include "common/constants.h"
#include "mem/segments.h"

seg_desc_t gdt[NUM_GDT_ENTRIES];

descr_t gdtr = {.limit = sizeof(gdt) - 1, .base_addr = (unsigned int)&gdt};

extern void gdt_load(unsigned int);

static void
gdt_set_entry (
  int          num,
  unsigned int base_addr,
  unsigned int limit,
  uint8_t      low_flags,
  uint8_t      hi_flags
) {
  num                /= sizeof(seg_desc_t);
  gdt[num].low_limit  = limit & 0xFFFF;
  gdt[num].low_base   = base_addr & 0xFFFFFF;
  gdt[num].low_flags  = low_flags;
  gdt[num].hi_limit   = (limit >> 16) & 0x0F;
  gdt[num].hi_flags   = hi_flags;
  gdt[num].hi_base    = (base_addr >> 24) & 0xFF;
}

void
gdt_init (void) {
  unsigned char low_flags;

  gdt_set_entry(0, 0, 0, 0, 0);

#define SEG_ARGS 0, FOUR_GB, low_flags, SD_OPSIZE32 | SD_PAGE4KB

  low_flags = SD_CODE | SD_CD | SD_DPL0 | SD_PRESENT;
  gdt_set_entry(KERNEL_CS, SEG_ARGS);
  low_flags = SD_DATA | SD_CD | SD_DPL0 | SD_PRESENT;
  gdt_set_entry(KERNEL_DS, SEG_ARGS);

  low_flags = SD_CODE | SD_CD | SD_DPL3 | SD_PRESENT;
  gdt_set_entry(USER_CS, SEG_ARGS);
  low_flags = SD_DATA | SD_CD | SD_DPL3 | SD_PRESENT;
  gdt_set_entry(USER_DS, SEG_ARGS);

#undef SEG_ARGS

  low_flags = SD_TSS_PRESENT;
  gdt_set_entry(TSS, 0, sizeof(i386tss_t), low_flags, SD_OPSIZE32);

  gdt_load((unsigned int)&gdtr);
}
