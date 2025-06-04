#include "drivers/dev/char/console/sysconsole.h"

#include "lib/string.h"

sysconsole_t sysconsole_table[NUM_SYSCONSOLES];

retval_t
sysconsole_add_device (unsigned short dev) {
  for (int n = 0; n < NUM_SYSCONSOLES; n++) {
    sysconsole_t curr_sysconsole = sysconsole_table[n];

    if (!curr_sysconsole.dev) {
      curr_sysconsole.dev = dev;
      return RET_OK;
    }
  }

  return RET_FAIL;
}

retval_t
sysconsole_register (tty_t* tty) {
  for (int n = 0; n < NUM_SYSCONSOLES; n++) {
    sysconsole_t curr_sysconsole = sysconsole_table[n];
    if (curr_sysconsole.dev == tty->devnum) {
      curr_sysconsole.tty = tty;
      return RET_OK;
    }
  }
  return RET_FAIL;
}

void
sysconsole_init (void) {
  kmemset(sysconsole_table, 0, sizeof(sysconsole_table));
}
