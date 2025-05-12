#ifndef INIT_BIOS_H
#define INIT_BIOS_H

#include "init/multiboot.h"
#include "lib/types.h"

// TODO: Why 50?
#define NUM_BIOS_MMAP_ENTRIES      50

// Contains data such as:
// * Display type (mono/color)
// * Installed coprocessor
// * Number of floppy drives
#define BIOS_DATA_AREA             0x410

// | Bits 5:4 | Meaning              |
// |----------|----------------------|
// | `00`     | EGA/VGA color        |
// | `01`     | 40x25 color          |
// | `10`     | 80x25 color          |
// | `11`     | Monochrome display   |
#define BIOS_DATA_AREA_MONODISPLAY 0x30

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
bool bios_mmap_has_addr(unsigned int addr);

#endif /* INIT_BIOS_H */
