#ifndef DEV_TTY_H
#define DEV_TTY_H

#include "common/types.h"

#define NUM_TTYS 16

typedef struct {
  deviceno_t devnum;
  short      count;
  void*      data;

} tty_t;

extern tty_t tty_table[];

tty_t*   tty_get(deviceno_t devnum);
retval_t tty_register(deviceno_t devnum);
void     tty_init(void);

#endif /* DEV_TTY_H */
