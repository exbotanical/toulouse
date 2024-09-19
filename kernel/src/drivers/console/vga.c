#include "drivers/console/vga.h"

#include "lib/string.h"

static const uint32_t VGA_WIDTH  = 80;
static const uint32_t VGA_HEIGHT = 25;

vga_console_t* global_vga_con    = NULL;

static inline uint8_t
vga_entry_color (vga_color_t fg, vga_color_t bg) {
  return fg | bg << 4;
}

static inline uint16_t
vga_entry (uint8_t uc, uint8_t color) {
  return (uint16_t)uc | (uint16_t)color << 8;
}

static void
vga_console_scroll (void) {
  for (uint32_t y = 1; y < VGA_HEIGHT; y++) {
    for (uint32_t x = 0; x < VGA_WIDTH; x++) {
      global_vga_con->buffer[(y - 1) * VGA_WIDTH + x] = global_vga_con->buffer[y * VGA_WIDTH + x];
    }
  }

  // Clear the last row
  for (uint32_t x = 0; x < VGA_WIDTH; x++) {
    global_vga_con->buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x]
      = vga_entry(' ', global_vga_con->color);
  }

  global_vga_con->row = VGA_HEIGHT - 1;
}

void
vga_globl_console_init (void) {
  global_vga_con->row    = 0;
  global_vga_con->col    = 0;
  global_vga_con->color  = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
  global_vga_con->buffer = (uint16_t*)VGA_BUFFER;

  vga_console_clear_screen();
}

void
vga_console_clear_screen (void) {
  for (uint32_t y = 0; y < VGA_HEIGHT; y++) {
    for (uint32_t x = 0; x < VGA_WIDTH; x++) {
      const uint32_t idx          = y * VGA_WIDTH + x;
      global_vga_con->buffer[idx] = vga_entry(' ', global_vga_con->color);
    }
  }
}

void
vga_console_setcolor (vga_color_t color) {
  global_vga_con->color = color;
}

void
vga_console_putchar_at (char c, uint32_t x, uint32_t y) {
  const uint32_t idx          = y * VGA_WIDTH + x;
  global_vga_con->buffer[idx] = vga_entry(c, global_vga_con->color);
}

void
vga_console_putchar (char c) {
  if (c == '\n') {
    global_vga_con->col = 0;
    if (++global_vga_con->row == VGA_HEIGHT) {
      vga_console_scroll();
    }
  } else {
    vga_console_putchar_at(c, global_vga_con->col, global_vga_con->row);
    if (++global_vga_con->col == VGA_WIDTH) {
      global_vga_con->col = 0;
      if (++global_vga_con->row == VGA_HEIGHT) {
        vga_console_scroll();
      }
    }
  }
}

void
vga_console_write (const char* data, uint32_t size) {
  for (uint32_t i = 0; i < size; i++) {
    vga_console_putchar(data[i]);
  }
}

void
vga_console_writestr (const char* data) {
  vga_console_write(data, k_strlen(data));
}
