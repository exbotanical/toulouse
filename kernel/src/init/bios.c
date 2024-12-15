#include "init/bios.h"

#include "drivers/console/vga.h"
#include "kernel.h"
#include "mem/paging.h"
#include "mem/segments.h"

bios_mmap_t bios_mmap[NUM_BIOS_MMAP_ENTRIES];
bios_mmap_t kernel_mmap[NUM_BIOS_MMAP_ENTRIES];

static char *bios_mem_type[]
  = {NULL, "available", "reserved", "ACPI Reclaim", "ACPI NVS", "unusable", "disabled"};

// Basically taken straight out of Fiwix, with some small modifications. See the licenses.
void
bios_mmap_init (multiboot_mmap_entry_t *mmap, unsigned int mmap_len) {
  struct multiboot_mmap_entry *bmmap;
  unsigned int                 from_high, from_low, to_high, to_low;
  unsigned long long           to, to_orig;
  int                          n, type;

  bmmap = mmap;

  if (bmmap) {
    n = 0;

    while ((unsigned int)bmmap < (unsigned int)mmap + mmap_len) {
      from_high = (unsigned int)(bmmap->addr >> 32);
      from_low  = (unsigned int)(bmmap->addr & 0xFFFFFFFF);
      to_orig   = (bmmap->addr + bmmap->len); /* preserve original end address */
      to        = (bmmap->addr + bmmap->len) - 1;
      to_high   = (unsigned int)(to >> 32);
      to_low    = (unsigned int)(to & 0xFFFFFFFF);
      type      = (int)bmmap->type;
      vgaprintf(
        "%s    0x%08x%08x-0x%08x%08x %s\n",
        n ? "      " : "memory",
        from_high,
        from_low,
        to_high,
        to_low,
        bios_mem_type[type]
      );
      // restore the original end address
      to_high = (unsigned int)(to_orig >> 32);
      to_low  = (unsigned int)(to_orig & 0xFFFFFFFF);
      if (n < NUM_BIOS_MMAP_ENTRIES && bmmap->len) {
        bios_mmap[n].from      = from_low;
        bios_mmap[n].from_high = from_high;
        bios_mmap[n].to        = to_low;
        bios_mmap[n].to_high   = to_high;
        bios_mmap[n].type      = type;
        // only memory addresses below 4GB are counted
        if (!from_high && !to_high) {
          if (type == MULTIBOOT_MEMORY_AVAILABLE) {
            from_low &= PAGE_MASK;
            to_low   &= PAGE_MASK;
          }
        }
        n++;
      }
      bmmap
        = (struct multiboot_mmap_entry *)((unsigned int)bmmap + bmmap->size + sizeof(bmmap->size));
    }

  } else {
    vgaprintf("%s\n", "[WARN]: your BIOS has not provided a memory map.");
    bios_mmap[0].from      = 0;
    bios_mmap[0].to        = k_param_memsize * 1024;
    bios_mmap[0].from_high = 0;
    bios_mmap[0].to_high   = 0;
    bios_mmap[0].type      = MULTIBOOT_MEMORY_AVAILABLE;
    bios_mmap[1].from      = 0x00100000;
    bios_mmap[1].to        = (k_param_extmemsize + 1024) * 1024;
    bios_mmap[1].from_high = 0;
    bios_mmap[1].to_high   = 0;
    bios_mmap[1].type      = MULTIBOOT_MEMORY_AVAILABLE;
  }

  k_memcpy(kernel_mmap, bios_mmap, NUM_BIOS_MMAP_ENTRIES * sizeof(bios_mmap_t));
}
