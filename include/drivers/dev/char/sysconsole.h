#ifndef DRIVERS_DEV_CHAR_SYSCONSOLE_H
#define DRIVERS_DEV_CHAR_SYSCONSOLE_H

#include "drivers/dev/char/tty.h"
#include "drivers/dev/device.h"
#include "kconfig.h"
#include "lib/types.h"

typedef struct {
  device_t dev;
  tty_t*   tty;
} sysconsole_t;

extern sysconsole_t sysconsole_table[NUM_SYSCONSOLES];

retval_t sysconsole_add_device(device_t d);
retval_t sysconsole_register(tty_t* tty);
void     sysconsole_init(void);

#endif /* DRIVERS_DEV_CHAR_SYSCONSOLE_H */
