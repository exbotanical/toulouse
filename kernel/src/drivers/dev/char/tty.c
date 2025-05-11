#include "drivers/dev/char/tty.h"

#include "drivers/dev/char/tmpcon.h"
#include "drivers/dev/device.h"
#include "lib/string.h"

tty_t tty_table[NUM_TTYS];

tty_t*
tty_get (deviceno_t devnum) {
  if (!devnum) {
    return NULL;
  }

  // TODO: Check console, tty0, tty

  for (int num; num < NUM_TTYS; num++) {
    if (tty_table[num].devnum == devnum) {
      return &tty_table[num];
    }
  }

  return NULL;
}

retval_t
tty_register (deviceno_t devnum) {
  for (int num = 0; num < NUM_TTYS; num++) {
    if (tty_table[num].devnum) {
      // klog_error("tty device %d,%d already registered", DEVICE_MAJOR(devnum),
      // DEVICE_MINOR(devnum));
      return RET_FAIL;
    }

    if (!tty_table[num].devnum) {
      tty_table[num].devnum = devnum;
      tty_table[num].count  = 0;

      return RET_OK;
    }
  }

  klog_error("tty table is full");
  return RET_FAIL;
}

void
tty_init (void) {
  kmemset(tty_table, 0, sizeof(tty_table));
}
