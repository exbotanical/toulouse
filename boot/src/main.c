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
  elf32_header *elf = (elf32_header *)TMP_ELF_LOCATION;

  // Read the first page on disk
  ata_read_segment((uint8_t *)elf, 8 * ATA_SECTOR_SZ, 0);
  clear_screen();

  if (elf->e_magic != ELF_MAGIC) {
    vga_print(111);

  } else {
    vga_print(222);
  }
}
