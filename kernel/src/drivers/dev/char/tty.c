#include "drivers/dev/char/tty.h"

#include "lib/string.h"

tty_t tty_table[NUM_TTYS];

void
tty_init (void) {
  kmemset(tty_table, 0, sizeof(tty_table));
}
