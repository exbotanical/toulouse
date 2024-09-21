#ifndef MEM_BASE_H
#define MEM_BASE_H

#include "common/types.h"
#include "lib/math.h"

// TODO: Base addresses should be shared with linker script
#define KERNEL_PHYSICAL_BASE 0x00100000
#define KERNEL_VIRTUAL_BASE  0xC0000000

#define KERNEL_NUM_TABLES    256
#define NUM_ENTRIES          1024
#define PAGE_SZ              4096

typedef uint32_t phys_addr_t;

#endif /* MEM_BASE_H */
