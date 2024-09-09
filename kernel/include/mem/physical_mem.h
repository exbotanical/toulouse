#ifndef PHYSICAL_MEM_H
#define PHYSICAL_MEM_H

#include "common/types.h"

#define PHYSM_ALLOC_BLOCKS_PER_BYTE 8
#define PHYSM_ALLOC_BLOCK_SZ        4096
#define PHYSM_ALLOC_BLOCK_ALIGN     PHYSM_ALLOC_BLOCK_SZ

// Size of physical memory
static uint32_t phys_alloc_mem_sz      = 0;

// Number of blocks currently in use
static uint32_t phys_alloc_used_blocks = 0;

// Max number of available memory blocks
static uint32_t phys_alloc_max_blocks  = 0;

// Memory map bit array, where each bit represents a single memory block.
static uint32_t* phys_alloc_mmap       = 0;

static inline void
phys_alloc_set (uint32_t bit) {
  // Divide by 32 to get the correct index in the map array.
  // Modulo selects the position of the bit in the selected uint32_t.
  // Left shift, setting the selected position to 1.
  phys_alloc_mmap[bit / 32] |= (1 << (bit % 32));
}

static inline void
phys_alloc_unset (uint32_t bit) {
  phys_alloc_mmap[bit / 32] &= ~(1 << (bit % 32));
}

static inline void
phys_alloc_isset (uint32_t bit) {
  phys_alloc_mmap[bit / 32] & (1 << (bit % 32));
}

int32_t phys_alloc_first_free(void);
void    phys_alloc_init(const uint32_t start_addr, const uint32_t sz);
void    phys_alloc_region_init(const uint32_t base, const uint32_t sz);
void*   phys_alloc_alloc_block(void);
void    phys_alloc_free_block(void* p);

#endif /* PHYSICAL_MEM_H */
