#ifndef DRIVER_DEV_CHAR_TTY_H
#define DRIVER_DEV_CHAR_TTY_H

#include "drivers/dev/char/charq.h"
#include "lib/types.h"

#define NUM_TTYS 16

typedef enum {
  /**
   * Indicates raw mode is enabled.
   * This tells the kernel to pass the raw hardware scancodes
   * directly to userspace without interpretation.
   */
  KBD_MODE_RAW    = 0x00,

  /**
   * Indicates full translation mode is enabled.
   * This instructs the kernel to map keys to ASCII chars or sequences
   */
  KBD_MODE_XLATE  = 0x01,

  /**
   * Indicates medium raw mode is enabled.
   * Medium raw mode entails only minimal processing: scancode → logical keycode mapping, but no
   * full ASCII translation. Retains press/release info.
   */
  KBD_MODE_MEDRAW = 0x02
} keyboard_mode;

typedef struct {
  // TODO: enum?
  char mode;
} keyboard_state_t;

typedef struct {
  deviceno_t devnum;
  short      count;
  void      *data;

  keyboard_state_t kbd_state;

  charq_t readq;

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

#endif /* DRIVER_DEV_CHAR_TTY_H */
