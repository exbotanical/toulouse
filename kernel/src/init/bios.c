#include "init/bios.h"

#include "common/constants.h"
#include "drivers/console/tmpcon.h"
#include "kernel.h"
#include "kstat.h"
#include "mem/paging.h"
#include "mem/segments.h"

bios_mmap_t bios_mmap[NUM_BIOS_MMAP_ENTRIES];
bios_mmap_t kernel_mmap[NUM_BIOS_MMAP_ENTRIES];

static char *bios_mem_type[]
  = {NULL, "available", "reserved", "ACPI Reclaim", "ACPI NVS", "unusable", "disabled"};

void
bios_mmap_init (multiboot_mmap_entry_t *mmap, unsigned int mmap_len) {
  // We need to fill out a data structure using the multiboot info supplied to us by the bootloader.
  // Where does the multiboot memory stuff actually come from? Primarily the E820 memory map.
  // GRUB, for example, queries the BIOS  using the INT 0x15 interrupt with EAX = 0xE820, which
  // returns a list of memory regions.

  // See: https://en.wikipedia.org/wiki/E820
  multiboot_mmap_entry_t *bmmap;
  unsigned int            from_high, from_low, to_high, to_low;
  unsigned long long      to, to_orig;
  int                     n, type;

  bmmap                = mmap;
  kstat.physical_pages = 0;

  if (bmmap) {
    n = 0;
    // Walk the memory map
    while ((unsigned int)bmmap < (unsigned int)mmap + mmap_len) {
      // Extract the address fields. We're effectively splitting the 64-bit address fields into high
      // and low 32-bit parts
      from_high = (unsigned int)(bmmap->addr >> 32);
      from_low  = (unsigned int)(bmmap->addr & FOUR_GB);
      to_orig   = (bmmap->addr + bmmap->len);  // preserve original end address
      to        = (bmmap->addr + bmmap->len) - 1;
      to_high   = (unsigned int)(to >> 32);
      to_low    = (unsigned int)(to & FOUR_GB);
      type      = (int)bmmap->type;

      kprintf(
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
      to_low  = (unsigned int)(to_orig & FOUR_GB);

      // Only process if we're dealing with a non-zero memory region and won't overflow the bounds
      // of `bios_mmap`
      if (n < NUM_BIOS_MMAP_ENTRIES && bmmap->len) {
        // Store the memory offsets
        bios_mmap[n].from      = from_low;
        bios_mmap[n].from_high = from_high;
        bios_mmap[n].to        = to_low;
        bios_mmap[n].to_high   = to_high;
        bios_mmap[n].type      = type;

        // Count available physical pages
        // Only memory addresses below 4GB are counted
        if (!from_high && !to_high) {
          if (type == MULTIBOOT_MEMORY_AVAILABLE) {
            from_low &= PAGE_MASK;
            to_low   &= PAGE_MASK;

            // the first MB is not counted here
            if (from_low >= 0x100000) {
              kstat.physical_pages += (to_low - from_low) / PAGE_SIZE;
            }
          }
        }
        n++;
      }
      // Jump the the next entry by using `size` to skip over the rest of the  current entry, plus 4
      // bytes for the `size` field itself.
      bmmap = (multiboot_mmap_entry_t *)((unsigned int)bmmap + bmmap->size + sizeof(bmmap->size));
    }
    // Add 1 MB worth of pages manually
    kstat.physical_pages += (1024 >> 2);
    // Cap memory to kernel's address space limit
    if (kstat.physical_pages > (GDT_BASE >> PAGE_SHIFT)) {
      klogf_warn(
        "detected a total of %dMB of available memory below 4GB.\n",
        (kstat.physical_pages << 2) / 1024
      );
    }
    // Fallback to a default memory map if none was supplied by the bootloader
  } else {
    klog_warn("your BIOS has not provided a memory map.");
    bios_mmap[0].from      = 0;
    bios_mmap[0].to        = kstat.param.memsize * 1024;
    bios_mmap[0].from_high = 0;
    bios_mmap[0].to_high   = 0;
    bios_mmap[0].type      = MULTIBOOT_MEMORY_AVAILABLE;
    bios_mmap[1].from      = 0x00100000;
    bios_mmap[1].to        = (kstat.param.extmemsize + 1024) * 1024;
    bios_mmap[1].from_high = 0;
    bios_mmap[1].to_high   = 0;
    bios_mmap[1].type      = MULTIBOOT_MEMORY_AVAILABLE;
    kstat.physical_pages   = (kstat.param.extmemsize + 1024) >> 2;
  }

  // Truncate physical memory to upper kernel address space size (1GB or 2GB), since
  // currently all memory is permanently mapped there.
  if (kstat.physical_pages > (GDT_BASE >> PAGE_SHIFT)) {
    kstat.physical_pages = (GDT_BASE >> PAGE_SHIFT);
    klogf_warn("only up to %dGB of physical memory will be used.\n", GDT_BASE >> 30);
  }

  kmemcpy(kernel_mmap, bios_mmap, NUM_BIOS_MMAP_ENTRIES * sizeof(bios_mmap_t));
}

bool
bios_mmap_has_addr (unsigned int addr) {
  int          ret = false;
  bios_mmap_t *bmm = &kernel_mmap[0];

  for (unsigned int n = 0; n < NUM_BIOS_MMAP_ENTRIES; n++, bmm++) {
    if (bmm->to && bmm->type == MULTIBOOT_MEMORY_AVAILABLE && !bmm->from_high && !bmm->to_high) {
      if (addr >= bmm->from && addr < (bmm->to & PAGE_MASK)) {
        ret = true;
      }
    }
  }

  // A second pass is necessary because the array isn't sorted
  bmm = &kernel_mmap[0];
  for (unsigned int n = 0; n < NUM_BIOS_MMAP_ENTRIES; n++, bmm++) {
    if (bmm->to && bmm->type == MULTIBOOT_MEMORY_RESERVED && !bmm->from_high && !bmm->to_high) {
      if (addr >= bmm->from && addr < (bmm->to & PAGE_MASK)) {
        ret = false;
      }
    }
  }

  return ret;
}
