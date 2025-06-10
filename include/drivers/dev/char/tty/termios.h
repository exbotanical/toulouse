#ifndef DRIVER_DEV_CHAR_TERMIOS_H
#define DRIVER_DEV_CHAR_TERMIOS_H

#include "drivers/dev/char/tty/termbits.h"

#define NUM_CTRL_CHARS     8
#define NUM_CTRL_CHARS_STD 19
struct tty;

typedef struct {
  unsigned short int ws_row;
  unsigned short int ws_col;
  unsigned short int ws_xpixel;
  unsigned short int ws_ypixel;
} winsize_t;

/**
 * Legacy terminal control structure
 */
typedef struct {
  unsigned short int c_iflag;              /* Input mode flags */
  unsigned short int c_oflag;              /* Output mode flags */
  unsigned short int c_cflag;              /* Control mode flags */
  unsigned short int c_lflag;              /* Local mode flags */
  unsigned char      c_line;               /* Line discipline */
  unsigned char      c_cc[NUM_CTRL_CHARS]; /* Control characters */
} termio_t;

/**
 * New terminal control structure.
 * Houses configuration settings that control how terminal I/O is processed
 */
typedef struct {
  tcflag_t c_iflag;                  /* Input mode flags */
  tcflag_t c_oflag;                  /* Output mode flags */
  tcflag_t c_cflag;                  /* Control mode flags */
  tcflag_t c_lflag;                  /* Local mode flags */
  cc_t     c_line;                   /* Line discipline */
  cc_t     c_cc[NUM_CTRL_CHARS_STD]; /* Control characters */
} termios_t;

/**
 * Converts a given termios into a termio (stored in a given pointer)
 */
void termios_to_termio(termios_t *termios, termio_t *termio);

/**
 * Converts a given termio into a termios (stored in a given pointer)
 */
void termio_to_termios(termio_t *termio, termios_t *termios);

/**
 * Resets the termios structure on the given tty instance to defaults
 */
void termios_reset(struct tty *tty);

#endif /* DRIVER_DEV_CHAR_TERMIOS_H */
