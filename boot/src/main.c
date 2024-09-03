#include "arch/x86.h"
#include "boot.h"
#include "drivers/disk/ata.h"
#include "fs/elf.h"

#define TMP_ELF_LOCATION 0x10000

#define VGA_BUFFER       0xB8000
#define VGA_WIDTH        80
#define VGA_ATTRIBUTE    0x0F
#define VGA_HEIGHT       25

// tmp
static void
clear_screen (void) {
  volatile uint16_t *vga   = (volatile uint16_t *)VGA_BUFFER;
  uint16_t           blank = ' ' | (VGA_ATTRIBUTE << 8);

  for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
    vga[i] = blank;
  }
}

// tmp
static void
vga_print (unsigned int num) {
  volatile uint16_t *vga      = (volatile uint16_t *)VGA_BUFFER;
  int                position = 0;

  if (num == 0) {
    vga[position++] = ('0' | (VGA_ATTRIBUTE << 8));
    return;
  }

  char buffer[10];
  int  i = 0;
  while (num > 0) {
    buffer[i++]  = '0' + (num % 10);
    num         /= 10;
  }

  while (i > 0) {
    vga[position++] = (buffer[--i] | (VGA_ATTRIBUTE << 8));
  }
}

void
load_kernel (void) {
  clear_screen();

  elf32_hdr *elf = (elf32_hdr *)TMP_ELF_LOCATION;
  // Read the elf header into memory
  ata_read_segment((uint8_t *)elf, 4096, 0);

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
