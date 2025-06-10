#include "debug/logger.h"

#include "drivers/dev/char/charq.h"
#include "drivers/dev/char/tty/tty.h"

static unsigned int log_count;

void
flush_log_buf (tty_t *tty) {
  char *buffer = &log_buf[0];
  int   count  = log_count;

  while (count) {
    if (charq_put_char(&tty->write_q, *buffer) < 0) {
      tty->output(tty);
      continue;
    }

    count--;
    buffer++;
  }

  tty->output(tty);
}
