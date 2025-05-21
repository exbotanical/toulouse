#ifndef DEV_TTY_H
#define DEV_TTY_H

#include "lib/types.h"

#define NUM_TTYS 16

typedef struct {
  deviceno_t devnum;
  short      count;
  void      *data;

  void (*stop)(struct tty *);
  void (*start)(struct tty *);
  void (*delete_tab)(struct tty *);
  void (*reset)(struct tty *);
  void (*input)(struct tty *);
  void (*output)(struct tty *);
  int (*open)(struct tty *);
  int (*close)(struct tty *);
  void (*set_termios)(struct tty *);
} tty_t;

extern tty_t tty_table[];

tty_t   *tty_get(deviceno_t devnum);
retval_t tty_register(deviceno_t devnum);
void     tty_init(void);

#endif /* DEV_TTY_H */
