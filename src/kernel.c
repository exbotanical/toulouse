#include "kernel.h"

#include <stdint.h>
#include <stddef.h>

void putchar (int x, int y, char c, char color);

uint16_t charconv (char c, char color);

uint16_t* video_mem = 0;
uint16_t terminal_row = 0;
uint16_t terminal_col = 0;

void term_init (void) {
  video_mem = (uint16_t*)(0xB8000);

  for (int y = 0; y < VGA_HEIGHT; y++) {
    for (int x = 0; x < VGA_WIDTH; x++) {
      putchar(x, y, ' ', 0);
    }
  }
}

uint16_t charconv (char c, char color) {
  return (color << 8) | c;
}

size_t strlen (const char* str) {
  size_t len = 0;

  while (str[len]) {
    len++;
  }

  return len;
}

void putchar (int x, int y, char c, char color) {
  video_mem[(y * VGA_WIDTH) + x] = charconv(c, color);
}

void writechar (char c, char color) {
  if (c == '\n') {
    terminal_row++;
    terminal_col = 0;
  } else {
    putchar(terminal_col, terminal_row, c, color);
    terminal_col++;

    if (terminal_col >= VGA_WIDTH) {
      terminal_col = 0;
      terminal_row++;
    }
  }
}

void print (const char* str) {
  size_t len = strlen(str);

  for (int i = 0; i < len; i++) {
    writechar(str[i], 9);
  }
}

void kernel_main (void) {
  term_init();
  print("Welcome to Toulouse\n");
}
