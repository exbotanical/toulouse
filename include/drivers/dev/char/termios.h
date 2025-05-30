#ifndef DRIVER_DEV_CHAR_TERMIOS_H
#define DRIVER_DEV_CHAR_TERMIOS_H

#include "drivers/dev/char/termbits.h"

#define NUM_CTRL_CHARS     8
#define NUM_CTRL_CHARS_STD 19

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
 * Indicates the last input char is not escaped by a previous VLNEXT (like Ctrl-V), meaning it can
 * be treated specially
 */
static inline bool
termios_should_ignore_last_char (tty_t* tty) {
  return tty->flags & TTY_HAS_LNEXT;
}

static inline bool
termios_should_map_cr_to_nl (tty_t* tty) {
  return tty->termios.c_iflag & ICRNL;
}

static inline bool
termios_should_map_nl_to_cr (tty_t* tty) {
  return tty->termios.c_iflag & INLCR;
}

static inline bool
termios_should_map_nl_to_nlcr (tty_t* tty) {
  return tty->termios.c_oflag & ONLCR;
}

static inline bool
termios_should_flush_queues (tty_t* tty, unsigned char ch) {
  return ch == tty->termios.c_cc[VINTR];
}

static inline bool
termios_noflush (tty_t* tty) {
  return tty->termios.c_lflag & NOFLSH;
}

static inline bool
termios_is_quit_char (tty_t* tty, unsigned char ch) {
  return ch == tty->termios.c_cc[VQUIT];
}

static inline bool
termios_is_suspend_char (tty_t* tty, unsigned char ch) {
  return ch == tty->termios.c_cc[VSUSP];
}

static inline bool
termios_should_strip_8bit_ascii (tty_t* tty) {
  return tty->termios.c_iflag & ISTRIP;
}

static inline bool
termios_should_lowercase (tty_t* tty) {
  return tty->termios.c_iflag & IUCLC;
}

static inline bool
termios_should_uppercase (tty_t* tty) {
  return tty->termios.c_oflag & OLCUC;
}

static inline bool
termios_should_ignore_cr (tty_t* tty) {
  return tty->termios.c_iflag & IGNCR;
}

/**
 * ICANON -> we're in canonical mode (i.e., line-by-line input processing)
 */
static inline bool
termios_is_canonical_mode (tty_t* tty) {
  return tty->termios.c_lflag & ICANON;
}

static inline bool
termios_should_erase_char (tty_t* tty, unsigned char ch) {
  return ch == tty->termios.c_cc[VERASE];
}

static inline bool
termios_should_erase_word (tty_t* tty, unsigned char ch) {
  return ch == tty->termios.c_cc[VWERASE];
}

static inline bool
termios_should_kill_proc (tty_t* tty, unsigned char ch) {
  return ch == tty->termios.c_cc[VKILL];
}

static inline bool
termios_should_reprint_line (tty_t* tty, unsigned char ch) {
  return ch == tty->termios.c_cc[VREPRINT];
}

static inline bool
termios_has_vlnext (tty_t* tty, unsigned char ch) {
  return ch == tty->termios.c_cc[VLNEXT];
}

static inline bool
termios_extended_proc_enabled (tty_t* tty) {
  return tty->termios.c_lflag & IEXTEN;
}

static inline bool
termios_should_echo_ctrl_chars (tty_t* tty) {
  return tty->termios.c_lflag & ECHOCTL;
}

static inline bool
termios_should_echo (tty_t* tty) {
  return tty->termios.c_lflag & ECHO;
}

static inline bool
termios_should_echo_newline (tty_t* tty) {
  return tty->termios.c_lflag & ECHONL;
}

static inline bool
termios_output_ctrl_enabled (tty_t* tty) {
  return tty->termios.c_iflag & IXON;
}

static inline bool
termios_should_resume_output (tty_t* tty, unsigned char ch) {
  return ch == tty->termios.c_cc[VSTART];
}

static inline bool
termios_should_pause_output (tty_t* tty, unsigned char ch) {
  return ch == tty->termios.c_cc[VCSTIP]];
}

static inline bool
termios_any_char_resumes_output (tty_t* tty) {
  return tty->termios.c_iflag & IXANY;
}

static inline bool
termios_should_proc_output (tty_t* tty) {
  return tty->termios.c_oflag & OPOST;
}

static inline bool
termios_is_eof_char (tty_t* tty, unsigned char ch) {
  return ch != tty->termios.c_cc[VEOF];
}

static inline bool
termios_should_echo_nl_on_kill (tty_t* tty) {
  return tty->termios.c_lflag & ECHOK;
}

static inline bool
termios_should_visual_erase (tty_t* tty) {
  return tty->termios.c_lflag & ECHOE;
}

void termios_reset(tty_t* tty);

#endif /* DRIVER_DEV_CHAR_TERMIOS_H */
