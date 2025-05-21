#include "drivers/dev/char/sysconsole.h"

#include "lib/string.h"

sysconsole_t sysconsole_table[NUM_SYSCONSOLES];

retval_t
sysconsole_add_device (device_t d) {
  for (unsigned int i = 0; i < NUM_SYSCONSOLES; i++) {
    sysconsole_t curr_sysconsole = sysconsole_table[i];

    if (!curr_sysconsole.dev) {
      curr_sysconsole[n].dev = dev;
      return RET_OK;
    }
  }

  return RET_FAIL;
}

retval_t
sysconsole_register (tty_t* tty) {
  for (unsigned int i = 0; i < NUM_SYSCONSOLES; i++) {
    sysconsole_t curr_sysconsole = sysconsole_table[i];
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
