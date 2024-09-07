#include "kmain.h"

#include "drivers/console/vga.h"

typedef struct smap_entry {
  uint64_t base_addr;
  uint64_t len;
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

void print_physical_memory_info(multiboot_info_t *mbi);
void print_smap(uint32_t len, uint32_t addr);

void
kmain (multiboot_info_t *mbi) {
  vga_globl_console_init();
  // uint32_t mmapl = *(uint32_t *)0xA500;
  char s[100];
  itoa(mbi->mmap_length, s, 10);
  vga_console_writestr(global_vga_con, "LENGTH: ");
  vga_console_writestr(global_vga_con, s);

  print_physical_memory_info(mbi);
  // print_smap(mmapl, 0xA504);
}

void
print_smap (uint32_t len, uint32_t addr) {
  smap_entry_t *mmaps = (smap_entry_t *)addr;

  char s[100];
  for (uint32_t i = 0; i < len; i++) {
    smap_entry_t mmap = mmaps[i];

    itoa(i, s, 10);
    vga_console_writestr(global_vga_con, "Region: ");
    vga_console_writestr(global_vga_con, s);
    vga_console_writestr(global_vga_con, "\n");

    itoa(mmap.acpi, s, 10);
    vga_console_writestr(global_vga_con, " acpi: ");
    vga_console_writestr(global_vga_con, s);
    vga_console_writestr(global_vga_con, "\n");

    itoa(mmap.type, s, 10);
    vga_console_writestr(global_vga_con, " type: ");
    vga_console_writestr(global_vga_con, s);
    vga_console_writestr(global_vga_con, "\n");

    itoa(mmap.len, s, 10);
    vga_console_writestr(global_vga_con, " len: ");
    vga_console_writestr(global_vga_con, s);
    vga_console_writestr(global_vga_con, "\n");

    itoa(mmap.base_addr, s, 10);
    vga_console_writestr(global_vga_con, " base_addr: ");
    vga_console_writestr(global_vga_con, s);
    vga_console_writestr(global_vga_con, "\n");

    vga_console_writestr(global_vga_con, "\n");
  }
}

void
print_physical_memory_info (multiboot_info_t *mbi) {
  for (uint32_t i = 0; i < mbi->mmap_length; i++) {
    multiboot_memory_map_t mmap = mbi->mmap[i];

    char s[100];
    if (i == 100) {
      break;
    }
    itoa(i, s, 10);
    vga_console_writestr(global_vga_con, "Region: ");
    vga_console_writestr(global_vga_con, s);
    vga_console_writestr(global_vga_con, "\n");

    itoa(mmap.addr, s, 10);
    vga_console_writestr(global_vga_con, " base: ");
    vga_console_writestr(global_vga_con, s);
    vga_console_writestr(global_vga_con, "\n");

    itoa(mmap.len, s, 10);
    vga_console_writestr(global_vga_con, " length: ");
    vga_console_writestr(global_vga_con, s);
    vga_console_writestr(global_vga_con, "\n");

    itoa(mmap.type, s, 10);
    vga_console_writestr(global_vga_con, " type: ");
    vga_console_writestr(global_vga_con, s);
    vga_console_writestr(global_vga_con, "\n");

    vga_console_writestr(global_vga_con, "\n");
  }
}
