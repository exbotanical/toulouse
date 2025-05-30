#ifndef DRIVER_DEV_CHAR_TTY_H
#define DRIVER_DEV_CHAR_TTY_H

#include "drivers/dev/char/charq.h"
#include "drivers/dev/char/termios.h"
#include "lib/types.h"

#define NUM_TTYS     16

#define VT_GETMODE   0x5601 /* Get mode of active vt */
#define VT_SETMODE   0x5602 /* Set mode of active vt */
#define VT_ACKACQ    0x02   /* Acknowledge switch */

#define TAB_SIZE     8      /* Number of literal spaces that comprise a tab */
#define MAX_TAB_COLS 132    /* Max number of tab stops */

#define LAST_CHAR(q) ((q)->tail ? (q)->tail->data[(q)->tail->next_write_index - 1] : '\0')

typedef enum {
  /**
   * The kernel switches terminals automatically without involving any userspace logic
   */
  VT_AUTO    = 0x00,
  /**
   * Userspace process handles switching. Kernel sends signals (relsig/acqsig) to notify the process
   * and waits for an acknowledgment (using VT_ACKACQ) before completing the switch
   */
  VT_PROCESS = 0x01
} vt_mode_mod;

/**
 * Defines how virtual terminal switching is handled, either automatically by the kernel or manually
 * by a userspace process.
 */
typedef struct {
  vt_mode_mod mode;  /* vt mode */
  char        waitv; /* If non-zero, write() operations block until the VT becomes active */
  short int relsig;  /* Signal to raise on release req. If in VT_PROCESS mode, the kernel sends this
                        signal to the controlling process when the VT is about to be switched away */
  short int acqsig;  /* Signal sent to the process when the VT is acquired again */
  short int frsig;   /* Unused; legacy field, always zero */
} vt_mode_t;

typedef struct {
  unsigned short int v_active; /* The number of the currently active virtual terminal */
  unsigned short int v_signal; /* The signal number that should be sent to the process managing the
                                  VT when switching occurs */
  unsigned short int v_state;  /* A bitmask representing which VTs are currently open (in use) */
} vt_stat_t;

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

typedef enum {
  /**
   * Indicates next character is escaped and should not be interpreted
   *
   * See: https://www.gnu.org/software/libc/manual/html_node/Other-Special.html
   */
  TTY_HAS_LNEXT = 0x01
} tty_flag;

typedef struct tty tty_t;

struct tty {
  deviceno_t devnum;
  short      count;
  void      *data;
  int        pid;

  keyboard_state_t kbd_state;

  /**
   * Indicates the canonical mode line has been terminated (by a newline) and is ready to be read
   */
  bool has_canon_ln;

  /**
   * Character queue for storing ready text.
   *
   * This is what the user-space program sees when it calls read() on the TTY.
   * When a full line is ready (in canonical mode), it's copied from cooked_q into read_q.
   * In raw or non-canonical mode, input may go straight into the read_q without line editing.
   */
  charq_t read_q;

  /**
   * Character queue for storing the current line being processed.
   *
   * This queue stores line-buffered input (i.e. "cooked mode" input).
   * This is where characters go when the terminal is in canonical mode.
   */
  charq_t cooked_q;

  /**
   * Character queue for holding characters that are being sent to the terminal screen.
   *
   * Example:
   * `charq_unputchar(&tty->cooked_q)` removes the last character from the line being edited.
   * If ECHO is on, three characters are added to write_q: \b, space, \b, to visually erase the
   * character.
   */
  charq_t write_q;

  termios_t termios;
  winsize_t win;

  vt_mode_t     vt_mode;
  unsigned char vc_mode;

  /**
   * Tracks the vconsole column we're on
   */
  int column;

  /**
   * An array of flags, where each index represents a column on the terminal screen. The boolean
   * value indicates whether this column is a tab stop.
   */
  bool tab_stop[MAX_TAB_COLS];

  void (*stop)(tty_t *);
  void (*start)(tty_t *);
  void (*delete_tab)(tty_t *);
  void (*reset)(tty_t *);
  void (*input)(tty_t *);
  void (*output)(tty_t *);
  int (*open)(tty_t *);
  int (*close)(tty_t *);
  void (*set_termios)(tty_t *);
};

extern tty_t tty_table[];

void     tty_cook_input(tty_t *tty);
tty_t   *tty_get(deviceno_t devnum);
retval_t tty_register(deviceno_t devnum);
void     tty_init(void);

#endif /* DRIVER_DEV_CHAR_TTY_H */
