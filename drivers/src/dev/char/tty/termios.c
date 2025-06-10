#include "drivers/dev/char/tty/termios.h"

#include "drivers/dev/char/keymap.h"
#include "drivers/dev/char/tty/tty.h"

void
termios_to_termio (termios_t *termios, termio_t *termio) {
  termio->c_iflag = termios->c_iflag;
  termio->c_oflag = termios->c_oflag;
  termio->c_cflag = termios->c_cflag;
  termio->c_lflag = termios->c_lflag;
  termio->c_line  = termios->c_line;

  for (int n = 0; n < NUM_CTRL_CHARS; n++) {
    termio->c_cc[n] = termios->c_cc[n];
  }
}

void
termio_to_termios (termio_t *termio, termios_t *termios) {
  termios->c_iflag = termio->c_iflag;
  termios->c_oflag = termio->c_oflag;
  termios->c_cflag = termio->c_cflag;
  termios->c_lflag = termio->c_lflag;
  termios->c_line  = termio->c_line;

  for (int n = 0; n < NUM_CTRL_CHARS; n++) {
    termios->c_cc[n] = termio->c_cc[n];
  }
}

void
termios_reset (tty_t *tty) {
  tty->kbd_state.mode         = KBD_MODE_XLATE;
  tty->termios.c_iflag        = ICRNL | IXON | IXOFF;
  tty->termios.c_oflag        = OPOST | ONLCR;
  tty->termios.c_cflag        = B9600 | CS8 | HUPCL | CREAD | CLOCAL;
  tty->termios.c_lflag        = ISIG | ICANON | ECHO | ECHOE | ECHOK | ECHOCTL | ECHOKE | IEXTEN;
  tty->termios.c_line         = 0;
  tty->termios.c_cc[VINTR]    = 3;  /* ^C */
  tty->termios.c_cc[VQUIT]    = 28; /* ^\ */
  tty->termios.c_cc[VERASE]   = BS; /* ^? (127) not '\b' (^H) */
  tty->termios.c_cc[VKILL]    = 21; /* ^U */
  tty->termios.c_cc[VEOF]     = 4;  /* ^D */
  tty->termios.c_cc[VTIME]    = 0;
  tty->termios.c_cc[VMIN]     = 1;
  tty->termios.c_cc[VSWTC]    = 0;
  tty->termios.c_cc[VSTART]   = 17;   /* ^Q */
  tty->termios.c_cc[VSTOP]    = 19;   /* ^S */
  tty->termios.c_cc[VSUSP]    = 26;   /* ^Z */
  tty->termios.c_cc[VEOL]     = '\n'; /* ^J */
  tty->termios.c_cc[VREPRINT] = 18;   /* ^R */
  tty->termios.c_cc[VDISCARD] = 15;   /* ^O */
  tty->termios.c_cc[VWERASE]  = 23;   /* ^W */
  tty->termios.c_cc[VLNEXT]   = 22;   /* ^V */
  tty->termios.c_cc[VEOL2]    = 0;
}
