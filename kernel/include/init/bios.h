#ifndef INIT_BIOS_H
#define INIT_BIOS_H

#include "init/multiboot.h"

// TODO: Why 50?
#define NUM_BIOS_MMAP_ENTRIES 50

typedef struct {
  unsigned int from;
  unsigned int from_high;
  unsigned int to;
  unsigned int to_high;
  int          type;
} bios_mmap_t;

extern bios_mmap_t bios_mmap[NUM_BIOS_MMAP_ENTRIES];
extern bios_mmap_t kernel_mmap[NUM_BIOS_MMAP_ENTRIES];

void bios_mmap_init(multiboot_mmap_entry_t* mmap, unsigned int mmap_len);

#endif /* INIT_BIOS_H */
