#ifndef MEM_BASE_H
#define MEM_BASE_H

#define KERNEL_PHYSICAL_BASE 0x100000
#define KERNEL_PAGE_OFFSET   0xC0000000
#define P2V(addr)            (addr < KERNEL_PAGE_OFFSET ? addr + KERNEL_PAGE_OFFSET : addr)
#define V2P(addr)            (addr - KERNEL_PAGE_OFFSET)

#ifndef ASM_SOURCE

#  include "common/types.h"

#  define KERNEL_BSS_SZ ((int)image_end - (int)data_end)

// https://wiki.osdev.org/Using_Linker_Script_Values#:~:text=A%20common%20problem%20is%20getting,a%20symbol%2C%20not%20a%20variable.
extern volatile char text_end[];
extern volatile char data_end[];
extern volatile char image_end[];

typedef uint32_t phys_addr_t;

#endif /* ASM_SOURCE */

#endif /* MEM_BASE_H */
