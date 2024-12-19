#include "drivers/console/vga.h"

#include <stdarg.h>

#include "lib/string.h"

#define WRITE_BUFFER_SZ 2048

static const uint32_t VGA_WIDTH     = 80;
static const uint32_t VGA_HEIGHT    = 25;

vga_console_t*       global_vga_con = NULL;
static vga_console_t glob;

static inline uint8_t
vga_entry_color (vga_color_t fg, vga_color_t bg) {
  return fg | bg << 4;
}

static inline uint16_t
vga_entry (uint8_t uc, uint8_t color) {
  return (uint16_t)uc | (uint16_t)color << 8;
}

static void
vga_console_scroll (vga_console_t* cons) {
  for (uint32_t y = 1; y < VGA_HEIGHT; y++) {
    for (uint32_t x = 0; x < VGA_WIDTH; x++) {
      cons->buffer[(y - 1) * VGA_WIDTH + x] = cons->buffer[y * VGA_WIDTH + x];
    }
  }

  // Clear the last row
  for (uint32_t x = 0; x < VGA_WIDTH; x++) {
    cons->buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', cons->color);
  }

  cons->row = VGA_HEIGHT - 1;
}

void
vga_global_console_init (void) {
  global_vga_con         = &glob;
  global_vga_con->row    = 0;
  global_vga_con->col    = 0;
  global_vga_con->color  = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
  global_vga_con->buffer = (uint16_t*)VGA_ADDR;

  vga_console_clear_screen(global_vga_con);
}

void
vga_console_clear_screen (vga_console_t* cons) {
  for (uint32_t y = 0; y < VGA_HEIGHT; y++) {
    for (uint32_t x = 0; x < VGA_WIDTH; x++) {
      const uint32_t idx = y * VGA_WIDTH + x;
      cons->buffer[idx]  = vga_entry(' ', cons->color);
    }
  }
  cons->row = 0;
  cons->col = 0;
}

void
vga_console_setcolor (vga_console_t* cons, vga_color_t color) {
  cons->color = color;
}

void
vga_console_putchar_at (vga_console_t* cons, char c, uint32_t x, uint32_t y) {
  const uint32_t idx = y * VGA_WIDTH + x;
  cons->buffer[idx]  = vga_entry(c, cons->color);
}

void
vga_console_putchar (vga_console_t* cons, char c) {
  if (c == '\n') {
    cons->col = 0;
    if (++cons->row == VGA_HEIGHT) {
      vga_console_scroll(cons);
    }
  } else {
    vga_console_putchar_at(cons, c, cons->col, cons->row);
    if (++cons->col == VGA_WIDTH) {
      cons->col = 0;
      if (++cons->row == VGA_HEIGHT) {
        vga_console_scroll(cons);
      }
    }
  }
}

void
vga_console_write (vga_console_t* cons, const char* data, uint32_t size) {
  for (uint32_t i = 0; i < size; i++) {
    vga_console_putchar(cons, data[i]);
  }
}

void
vga_printf (const char* fmt, ...) {
  va_list va;
  // TODO: global?
  char    buf[WRITE_BUFFER_SZ];

  va_start(va, fmt);
  sprintf_(buf, fmt, va);
  va_end(va);

  vga_console_write(global_vga_con, buf, k_strlen(buf));
}
