#ifndef TERMIO_H
#define TERMIO_H

#include <stdint.h>

#define VGA_ADDR   0xb8000
#define VGA_WIDTH  80
#define VGA_HEIGHT 20

void term_init(void);
void term_print(const char* s);

#endif /* TERMIO_H */
