#ifndef VGA_H
#define VGA_H

#include "common/types.h"

#define VGA_BUFFER 0xC03FF000

/* Hardware text mode color constants. */
typedef enum {
  VGA_COLOR_BLACK         = 0,
  VGA_COLOR_BLUE          = 1,
  VGA_COLOR_GREEN         = 2,
  VGA_COLOR_CYAN          = 3,
  VGA_COLOR_RED           = 4,
  VGA_COLOR_MAGENTA       = 5,
  VGA_COLOR_BROWN         = 6,
  VGA_COLOR_LIGHT_GREY    = 7,
  VGA_COLOR_DARK_GREY     = 8,
  VGA_COLOR_LIGHT_BLUE    = 9,
  VGA_COLOR_LIGHT_GREEN   = 10,
  VGA_COLOR_LIGHT_CYAN    = 11,
  VGA_COLOR_LIGHT_RED     = 12,
  VGA_COLOR_LIGHT_MAGENTA = 13,
  VGA_COLOR_LIGHT_BROWN   = 14,
  VGA_COLOR_WHITE         = 15,
} vga_color_t;

typedef struct {
  uint32_t  row;
  uint32_t  col;
  uint8_t   color;
  uint16_t* buffer;
} vga_console_t;

extern vga_console_t* global_vga_con;

void vga_globl_console_init(void);
void vga_console_clear_screen(vga_console_t* cons);
void vga_console_setcolor(vga_console_t* cons, vga_color_t color);
void vga_console_putchar_at(vga_console_t* cons, char c, uint32_t x, uint32_t y);
void vga_console_putchar(vga_console_t* cons, char c);
void vga_console_write(vga_console_t* cons, const char* data, uint32_t size);
void vga_console_writestr(vga_console_t* cons, const char* data);

#endif /* VGA_H */