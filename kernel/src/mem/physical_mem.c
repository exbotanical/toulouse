#include "mem/physical_mem.h"

#include "lib/k_string.h"

int32_t
phys_alloc_first_free (void) {
  for (uint32_t i = 0; i < phys_alloc_max_blocks / 32; i++) {
    // Check if all bits are set
    if (phys_alloc_mmap[i] != 0xFFFFFFFF) {
      // If not, we need to find the first non-set bit
      for (uint32_t j = 0; j < 32; j++) {
        if (!(phys_alloc_mmap[i] & (1 << j))) {
          return i * 32 + j;
        }
      }
    }
  }

  return -1;
}

void
phys_alloc_init (const uint32_t start_addr, const uint32_t sz) {
  phys_alloc_mem_sz      = sz;
  phys_alloc_mmap        = (uint32_t*)start_addr;
  phys_alloc_max_blocks  = sz / PHYSM_ALLOC_BLOCK_SZ;
  // Start off with every block reserved/used
  phys_alloc_used_blocks = phys_alloc_max_blocks;
  k_memset(phys_alloc_mmap, 0xFF, phys_alloc_max_blocks / PHYSM_ALLOC_BLOCKS_PER_BYTE);
}

void
phys_alloc_region_init (const uint32_t base, const uint32_t sz) {
  uint32_t align  = base / PHYSM_ALLOC_BLOCK_SZ;
  uint32_t blocks = sz / PHYSM_ALLOC_BLOCK_SZ;

  for (; blocks > 0; blocks--) {
    phys_alloc_unset(align++);
    phys_alloc_used_blocks--;
  }

  // First block is always set. This ensures allocs cannot be 0.
  phys_alloc_set(0);
}

void*
phys_alloc_alloc_block (void) {
  // TODO: track num free pgs
  int32_t frame = phys_alloc_first_free();

  if (frame == -1) {
    return 0;
  }

  phys_alloc_set(frame);

  uint32_t addr = frame * PHYSM_ALLOC_BLOCK_SZ;
  phys_alloc_used_blocks++;

  return (void*)addr;
}

void
phys_alloc_free_block (void* p) {
  uint32_t addr  = (uint32_t)p;
  uint32_t frame = addr / PHYSM_ALLOC_BLOCK_SZ;

  phys_alloc_unset(frame);

  phys_alloc_used_blocks--;
}
