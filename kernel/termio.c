#include "termio.h"

#include <stddef.h>

#include "std.h"

uint16_t* video_mem = (uint16_t*)(VGA_ADDR);
uint16_t term_row = 0;
uint16_t term_col = 0;

// Bits: ascii char code + color
// this is the same as storing videomem as char array and
// [0]=char_literal, [1]=color_digit
// Note we've reversed because intel is little endian
// e.g. 0x0341 -> 0x41, 0x03
static uint16_t to_term_char(char c, char color) { return (color << 8) | c; }

static void term_putchar(int x, int y, char c, char color) {
  video_mem[y * VGA_WIDTH + x] = to_term_char(c, color);
}

static void term_writechar(char c, char color) {
  if (c == '\n') {
    term_row++;
    term_col = 0;
    return;
  }

  term_putchar(term_col, term_row, c, color);
  term_col++;

  if (term_col >= VGA_WIDTH) {
    term_row++;
    term_col = 0;
  }
}

void term_init(void) {
  video_mem = (uint16_t*)(VGA_ADDR);
  term_row = 0;
  term_col = 0;

  for (unsigned int y = 0; y < VGA_HEIGHT; y++) {
    for (unsigned int x = 0; x < VGA_WIDTH; x++) {
      term_putchar(x, y, ' ', 0);
    }
  }
}

void term_print(const char* s) {
  size_t len = strlen(s);
  for (unsigned int i = 0; i < len; i++) {
    term_writechar(s[i], 15);
  }
}
