#ifndef DRIVER_DEV_CHAR_TTY_H
#define DRIVER_DEV_CHAR_TTY_H

#include "drivers/dev/char/charq.h"
#include "drivers/dev/char/tty/termios.h"
#include "fs/fd.h"
#include "fs/inode.h"
#include "lib/types.h"

#define NUM_TTYS     16  /* Number of TTYs supported by the kernel */

#define TAB_SIZE     8   /* Number of literal spaces that comprise a tab */
#define MAX_TAB_COLS 132 /* Max number of tab stops */

/**
 * Retrieves the last character from the given queue
 */
#define LAST_CHAR(q) ((q)->tail ? (q)->tail->data[(q)->tail->next_write_index - 1] : '\0')

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
  char mode; /* keyboard_mode */
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
  /**
   * Configuration flags
   */
  int flags;

  /**
   * How many live opens of this TTY there are
   */
  short open_count;

  /**
   * Attached device number
   */
  deviceno_t devnum;

  /**
   * A pointer to the device to which the TTY is attaches
   */
  void* data;

  /**
   * Associated process id
   */
  pid_t pid;

  /**
   * Associated foreground process group id
   */
  pid_t pgid;

  /**
   * Stores keyboard configurations such as the mode
   * TODO: rename -> kbd_conf
   */
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

  /**
   * I/O processing configurations and flags for this TTY
   */
  termios_t termios;

  /**
   * Window size details for this TTY
   */
  winsize_t win;

  /**
   * The terminal switching mode used by this TTY
   */
  vt_mode_t vt_mode;

  /**
   * Tracks the vconsole column we're on
   */
  int column;

  /**
   * An array of flags, where each index represents a column on the terminal screen. The boolean
   * value indicates whether this column is a tab stop.
   */
  bool tab_stop[MAX_TAB_COLS];

  /* Driver operations */
  void (*stop)(tty_t*);
  void (*start)(tty_t*);
  void (*delete_tab)(tty_t*);
  void (*reset)(tty_t*);
  void (*input)(tty_t*);
  void (*output)(tty_t*);
  int (*open)(tty_t*);
  int (*close)(tty_t*);
  void (*set_termios)(tty_t*);
};

/**
 * TODO:
 */
extern tty_t tty_table[];

/**
 * Indicates the last input char is not escaped by a previous VLNEXT (like Ctrl-V), meaning it can
 * be treated specially
 */
static inline bool
termios_should_ignore_last_char (tty_t* tty) {
  return tty->flags & TTY_HAS_LNEXT;
}

/**
 * Indicates we should translate carriage return to newline on input (unless IGNCR is set).
 */
static inline bool
termios_should_map_cr_to_nl (tty_t* tty) {
  return tty->termios.c_iflag & ICRNL;
}

/**
 * Indicates we should translate NL to CR on input
 */
static inline bool
termios_should_map_nl_to_cr (tty_t* tty) {
  return tty->termios.c_iflag & INLCR;
}

/**
 * Indicates we should map NL to CR-NL on output
 */
static inline bool
termios_should_map_nl_to_nlcr (tty_t* tty) {
  return tty->termios.c_oflag & ONLCR;
}

/**
 * Indicates the interrupt character (INTR). Send a SIGINT signal.
 * Recognized when ISIG is set, and then not passed as input
 */
static inline bool
termios_should_flush_queues (tty_t* tty, unsigned char ch) {
  return ch == tty->termios.c_cc[VINTR];
}

/**
 * Indicates we should disable flushing the input and output queues when generating signals for the
 * INT, QUIT, and SUSP characters
 */
static inline bool
termios_noflush (tty_t* tty) {
  return tty->termios.c_lflag & NOFLSH;
}

/**
 * Indicates the quit character (QUIT). Send SIGQUIT signal.
 * Recognized when ISIG is set, and then not passed as input.
 */
static inline bool
termios_is_quit_char (tty_t* tty, unsigned char ch) {
  return ch == tty->termios.c_cc[VQUIT];
}

/**
 * Indicates the suspend character (SUSP). Send SIGTSTP signal.
 * Recognized when ISIG is set, and then not passed as input.
 */
static inline bool
termios_is_suspend_char (tty_t* tty, unsigned char ch) {
  return ch == tty->termios.c_cc[VSUSP];
}

/**
 * Indicates we should strip off the eighth bit
 */
static inline bool
termios_should_strip_8bit_ascii (tty_t* tty) {
  return tty->termios.c_iflag & ISTRIP;
}

/**
 * Indicates we should map uppercase characters to lowercase on input
 */
static inline bool
termios_should_lowercase (tty_t* tty) {
  return tty->termios.c_iflag & IUCLC;
}

/**
 * Indicates we should map lowercase characters to uppercase on output
 */
static inline bool
termios_should_uppercase (tty_t* tty) {
  return tty->termios.c_oflag & OLCUC;
}

/**
 * Indicates we should ignore carriage return on input
 */
static inline bool
termios_should_ignore_cr (tty_t* tty) {
  return tty->termios.c_iflag & IGNCR;
}

/**
 * Indicates we're in canonical mode (i.e. line-by-line input processing)
 */
static inline bool
termios_is_canonical_mode (tty_t* tty) {
  return tty->termios.c_lflag & ICANON;
}

/**
 * Indicates the erase character (ERASE).
 * This erases the previous not-yet-erased character, but does not erase past EOF or
 * beginning-of-line. Recognized when ICANON is set, and then not passed as input.
 */
static inline bool
termios_should_erase_char (tty_t* tty, unsigned char ch) {
  return ch == tty->termios.c_cc[VERASE];
}

/**
 * Indicates word erase (WERASE).
 * Recognized when ICANON and IEXTEN are set, and then not passed as input
 */
static inline bool
termios_should_erase_word (tty_t* tty, unsigned char ch) {
  return ch == tty->termios.c_cc[VWERASE];
}

/**
 * Indicates the kill character (KILL).
 * This erases the input since the last EOF or beginning-of-line.
 * Recognized when ICANON is set, and then not passed as input.
 */
static inline bool
termios_should_kill_proc (tty_t* tty, unsigned char ch) {
  return ch == tty->termios.c_cc[VKILL];
}

/**
 * Indicates we must reprint unread characters (REPRINT).
 * Recognized when ICANON and IEXTEN are set, and then not passed as input.
 */
static inline bool
termios_should_reprint_line (tty_t* tty, unsigned char ch) {
  return ch == tty->termios.c_cc[VREPRINT];
}

/**
 * Indicates we've a literal next (LNEXT). Quotes the next input character, depriving it of a
 * possible special meaning.  Recognized when IEXTEN is set, and then not passed as input.
 */
static inline bool
termios_has_vlnext (tty_t* tty, unsigned char ch) {
  return ch == tty->termios.c_cc[VLNEXT];
}

/**
 * Indicates we should enable implementation-defined input processing.
 *
 * This flag, as well as ICANON must be enabled for the special characters EOL2, LNEXT, REPRINT,
 * WERASE to be interpreted, and for the IUCLC flag to be effective
 */
static inline bool
termios_extended_proc_enabled (tty_t* tty) {
  return tty->termios.c_lflag & IEXTEN;
}

/**
 * If ECHO is also set, indicates that terminal special characters other than TAB, NL, START, and
 * STOP should be echoed as ^X, where X is the character with ASCII code 0x40 greater than the
 * special character
 */
static inline bool
termios_should_echo_ctrl_chars (tty_t* tty) {
  return tty->termios.c_lflag & ECHOCTL;
}

/**
 * Indicates whether we should echo input characters
 */
static inline bool
termios_should_echo (tty_t* tty) {
  return tty->termios.c_lflag & ECHO;
}

/**
 * If ICANON is also set, indicates whether we should echo the NL character even if ECHO is not set
 */
static inline bool
termios_should_echo_newline (tty_t* tty) {
  return tty->termios.c_lflag & ECHONL;
}

/**
 * Indicates whether flow control is enabled
 */
static inline bool
termios_output_ctrl_enabled (tty_t* tty) {
  return tty->termios.c_iflag & IXON;
}

/**
 * Indicates whether we should restart output stopped by the Stop character
 */
static inline bool
termios_should_resume_output (tty_t* tty, unsigned char ch) {
  return ch == tty->termios.c_cc[VSTART];
}

/**
 * Indicates whether we should stop output until Start character typed
 */
static inline bool
termios_should_pause_output (tty_t* tty, unsigned char ch) {
  return ch == tty->termios.c_cc[VSTOP];
}

/**
 * Indicates whether typing any character will restart stopped output. (The default is to allow just
 * the START character to restart output.)
 */
static inline bool
termios_any_char_resumes_output (tty_t* tty) {
  return tty->termios.c_iflag & IXANY;
}

/**
 * Indicates whether we should perform output processing on the buferred text
 */
static inline bool
termios_should_proc_output (tty_t* tty) {
  return tty->termios.c_oflag & OPOST;
}

/**
 * Indicates whether the character a canonical end of file, meaning
 * this character causes the pending tty buffer to be sent to the waiting user program
 * without waiting for end-of-line.
 */
static inline bool
termios_is_eof_char (tty_t* tty, unsigned char ch) {
  return ch != tty->termios.c_cc[VEOF];
}

/**
 * Indicates whether the character is a canonical end of line
 */
static inline bool
termios_is_eol_char (tty_t* tty, unsigned char ch) {
  return ch != tty->termios.c_cc[VEOL];
}

/**
 * Indicates whether the character is a canonical end of line (variation 2)
 */
static inline bool
termios_is_eol2_char (tty_t* tty, unsigned char ch) {
  return ch != tty->termios.c_cc[VEOL2] && tty->termios.c_cc[VEOL2] != 0;
}

/**
 * If ICANON is also set, indicates that the KILL character erases the current line
 */
static inline bool
termios_should_echo_nl_on_kill (tty_t* tty) {
  return tty->termios.c_lflag & ECHOK;
}

/**
 * If ICANON is also set, indicates that the ERASE character erases the preceding input
 * character, and WERASE erases the preceding word
 */
static inline bool
termios_should_visual_erase (tty_t* tty) {
  return tty->termios.c_lflag & ECHOE;
}

/**
 * Indicates whether there's a timeout (in deciseconds) for noncanonical reads
 */
static inline bool
termios_has_noncanon_read_timeout (tty_t* tty) {
  return tty->termios.c_cc[VTIME] > 0;
}

/**
 * Indicates whether reads from the given tty must have a minumum number of characters that can be
 * read
 */
static inline bool
termios_has_min_read_batch_size (tty_t* tty) {
  return tty->termios.c_cc[VMIN];
}

// TODO: RELOCATE
/**
 * Indicates we should send the SIGTTOU signal to the process group of a background process which
 * tries to write to its controlling terminal
 */
static inline bool
termios_bg_proc_can_write_to_tty (tty_t* tty) {
  return tty->termios.c_lflag & TOSTOP;
}

/**
 * Dissociate all processes from the given controlling tty
 */
void tty_disassociate_ctty(tty_t* tty);

/**
 * Implements tty seek.
 * This isn't supported right now, so we always return `-ESPIPE`.
 */
long long int tty_llseek(inode_t* i, long long int offset);

/**
 * Runs canonical mode processing on input data, applying termios config updates
 */
void tty_cook_input(tty_t* tty);

/**
 * Reads count characters from a tty into a userspace buffer.
 *
 * Effectively, this is:
 * keyboard interrupt → read_q → [line discipline] → cooked_q → read() syscall → user buffer
 */
int tty_read(inode_t* i, fd_t* fd_table, char* buffer, size_t count);

/**
 * Writes count characters from a userspace buffer into a tty
 */
int tty_write(inode_t* i, fd_t* fd_table, const char* buffer, size_t count);

/**
 * Opens a tty on the given inode's attached device.
 * Attaches the tty to the current process if that process is the session leader, does not already
 * have a controlling tty, does not have a session id, and `O_NOCTTY` is not set.
 *
 * Returns 0 if successful or an errno.
 */
int tty_open(inode_t* i, fd_t* fd_table);

/**
 * Closes the tty on the given inode's attached device.
 * Remove this as controlling tty on all processes, if applicable.
 *
 * Returns 0 if successful or an errno.
 */
int tty_close(inode_t* i, fd_t* fd_table);

/**
 * Retrieves the TTY attached to the device with the given device number
 */
tty_t* tty_get(deviceno_t devnum);

/**
 * Performs a get_tty check on the device attached to the given inode. Returns a retval indicating
 * whether that tty exists under valid conditions.
 */
retval_t tty_select(inode_t* i, fs_tty_select_flag flag);

/**
 * Registers a new TTY in the TTY table
 */
retval_t tty_register(deviceno_t devnum);

/**
 * Initializes the global TTY table
 */
void tty_init(void);

#endif /* DRIVER_DEV_CHAR_TTY_H */
