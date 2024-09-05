#include "kmain.h"

#include "drivers/console/vga.h"

typedef struct smap_entry {
  uint32_t base_addr_lo;
  uint32_t base_addr_hi;
  uint32_t len_lo;
  uint32_t len_hi;
  uint32_t type;
  uint32_t acpi;
} __attribute__((packed)) smap_entry_t;

char *itoa(int value, char *buff, int base);

char *
itoa (int value, char *buff, int base) {
  char *charset = "0123456789abcdefghijklmnopqrstuvwxyz";
  char *ret     = buff;
  char  scratch[64];
  int   idx = 0;
  if (value < 0) {
    *buff++ = '-';
    value   = -value;
  }
  if (value == 0) {
    *buff++ = '0';
    *buff   = 0;
    return ret;
  }
  while (value > 0) {
    scratch[idx++]  = charset[value % base];
    value          /= base;
  }
  while (idx > 0) {
    *buff++ = scratch[--idx];
  }
  *buff = 0;
  return ret;
}

void
kmain (multiboot_info_t *mbi) {
  vga_globl_console_init();
  // vga_console_writestr(global_vga_con, "HELLO MOO");

  char s[100];
  itoa(mbi->mmap->len, s, 10);
  vga_console_writestr(global_vga_con, s);
}
