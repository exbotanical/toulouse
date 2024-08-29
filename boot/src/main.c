#include "boot.h"
#include "drivers/disk/ata.h"
#include "fs/elf.h"

#define TMP_ELF_LOCATION 0x10000

void
load_kernel (void) {
  elf32_header *elf = (elf32_header *)TMP_ELF_LOCATION;

  // Read the first page on disk
  ata_read_segment((uint8_t *)elf, 8 * ATA_SECTOR_SZ, 0);

  if (elf->e_magic != ELF_MAGIC) {
    asm("movb $'X', %al\n"
        "mov $0x0F, %ah\n"
        "mov %ax, (%edx)\n"
        "inc %edx");
  } else {
    asm("movb $'Y', %al\n"
        "mov $0x0F, %ah\n"
        "mov %ax, (%edx)\n"
        "inc %edx");
  }
}
