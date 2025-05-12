#include "arch/x86.h"
#include "boot.h"
#include "drivers/dev/block/ata.h"
#include "fs/elf.h"

#define TMP_ELF_LOCATION 0x10000

void
load_kernel (void) {
  elf32_hdr *elf = (elf32_hdr *)TMP_ELF_LOCATION;
  // Read the elf header into memory
  ata_read_segment((uint8_t *)elf, sizeof(elf32_hdr), 0);

  if (!elf_is_valid_executable(elf)) {
    // TODO: handle in bootloader
    return;
  }

  elf32_phdr *phdr = (elf32_phdr *)((uint8_t *)elf + elf->e_phoff);
  elf32_phdr *eph  = phdr + elf->e_phnum;

  for (; phdr < eph; phdr++) {
    uint8_t *pa = (uint8_t *)phdr->p_paddr;
    // Read in each loadable segment described by the program header
    ata_read_segment(pa, phdr->p_filesz, phdr->p_offset);

    // Zero out remaining memory if the segment's memory size is larger than
    // the segment's size in the file. Note a loadable segment may contain a
    // .bss section which contains uninitialized data. The ELF spec recommends
    // zeroing out the extra bytes of a segment's initialized memory in cases
    // where p_memsz > p_filesz.
    if (phdr->p_memsz > phdr->p_filesz) {
      stosb(pa + phdr->p_filesz, 0, phdr->p_memsz - phdr->p_filesz);
    }
  }

  // Jump to the kernel entrypoint
  void (*entry)(void) = (void (*)(void))(elf->e_entry);
  entry();
}
