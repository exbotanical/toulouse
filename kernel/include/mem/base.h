#ifndef MEM_BASE_H
#define MEM_BASE_H

#define KERNEL_PHYSICAL_BASE 0x100000
#define KERNEL_PAGE_OFFSET   0xC0000000

#ifndef ASM_SOURCE

#  include "common/types.h"

#  define KERNEL_BSS_SZ ((int)bss_end - (int)data_end)

extern char text_end[], data_end[], bss_end[];

typedef uint32_t phys_addr_t;

#endif /* ASM_SOURCE */

#endif /* MEM_BASE_H */
