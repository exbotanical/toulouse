#include "kmain.h"

#include "drivers/console/vga.h"

// TMP TODO: remove
void  print_physical_memory_info(multiboot_info_t *mbi);
void  print_smap(uint32_t len, uint32_t addr);
char *print_hex(uint32_t hex_num, char *hs);

void
kmain (multiboot_info_t *mbi) {
  vga_globl_console_init();

  char hex_string[80];
  print_hex(mbi->mmap_length, hex_string);
  vga_console_writestr(global_vga_con, "LENGTH: ");
  vga_console_writestr(global_vga_con, hex_string);

  print_physical_memory_info(mbi);
}

void
print_physical_memory_info (multiboot_info_t *mbi) {
  for (uint32_t i = 0; i < mbi->mmap_length; i++) {
    multiboot_memory_map_t mmap = mbi->mmap[i];

    char s[80];
    print_hex(i, s);
    vga_console_writestr(global_vga_con, "Region: ");
    vga_console_writestr(global_vga_con, s);
    vga_console_writestr(global_vga_con, "\n");

    print_hex(mmap.addr, s);
    vga_console_writestr(global_vga_con, " base: ");
    vga_console_writestr(global_vga_con, s);
    vga_console_writestr(global_vga_con, "\n");

    print_hex(mmap.len, s);
    vga_console_writestr(global_vga_con, " length: ");
    vga_console_writestr(global_vga_con, s);
    vga_console_writestr(global_vga_con, "\n");

    print_hex(mmap.type, s);
    vga_console_writestr(global_vga_con, " type: ");
    vga_console_writestr(global_vga_con, s);
    vga_console_writestr(global_vga_con, "\n");

    vga_console_writestr(global_vga_con, "\n");
  }
}

// Taken from quesofuego
char *
print_hex (uint32_t hex_num, char *hex_string) {
  char   *ascii_numbers = "0123456789ABCDEF";
  uint8_t nibble;
  uint8_t i   = 0, j, temp;
  uint8_t pad = 0;

  // If passed in 0, print a 0
  if (hex_num == 0) {
    // strncpy(hex_string, "0\0", 2);
    i = 1;
  }

  if (hex_num < 0x10) {
    pad = 1;  // If one digit, will pad out to 2 later
  }

  while (hex_num > 0) {
    // Convert hex values to ascii string
    nibble          = (uint8_t)hex_num & 0x0F;  // Get lowest 4 bits
    nibble          = ascii_numbers[nibble];    // Hex to ascii
    hex_string[i]   = nibble;                   // Move ascii char into string
    hex_num       >>= 4;                        // Shift right by 4 for next nibble
    i++;
  }

  if (pad) {
    hex_string[i++] = '0';  // Pad out string with extra 0
  }

  // Add initial "0x" to front of hex string
  hex_string[i++] = 'x';
  hex_string[i++] = '0';
  hex_string[i]   = '\0';  // Null terminate string

  // Number is stored backwards in hex_string, reverse the string by swapping ends
  //   until they meet in the middle
  i--;  // Skip null byte
  for (j = 0; j < i; j++, i--) {
    temp          = hex_string[j];
    hex_string[j] = hex_string[i];
    hex_string[i] = temp;
  }
  return hex_string;
}
