#ifndef SYSCALL_IOCTL_H
#define SYSCALL_IOCTL_H

// Note: 0x54 is just a magic number to make these relatively unique ('T')
#define TCGETS         0x5401   /* Get current termios settings */
#define TCSETS         0x5402   /* Set termios settings (immediate) */
#define TCSETSW        0x5403   /* Set termios settings after transmitting queued output */
#define TCSETSF        0x5404   /* Set termios settings after flushing input & output queues */
#define TCGETA         0x5405   /* Get legacy termios settings */
#define TCSETA         0x5406   /* Set legacy termios settings (immediate) */
#define TCSETAW        0x5407   /* Set legacy termios settings after output drains */
#define TCSETAF        0x5408   /* Set legacy termios settings after flushing queues */
#define TCSBRK         0x5409   /* Send break (0 = send, 1 = drain and flush) */
#define TCXONC         0x540A   /* Suspend or resume transmission (flow control) */
#define TCFLSH         0x540B   /* Flush input and/or output queue */
#define TIOCEXCL       0x540C   /* Set exclusive mode on tty (no further opens allowed) */
#define TIOCNXCL       0x540D   /* Clear exclusive mode */
#define TIOCSCTTY      0x540E   /* Set controlling terminal */
#define TIOCGPGRP      0x540F   /* Get foreground process group ID */
#define TIOCSPGRP      0x5410   /* Set foreground process group ID */
#define TIOCOUTQ       0x5411   /* Get count of bytes in output queue */
#define TIOCSTI        0x5412   /* Simulate typed input (inject a character into input queue) */
#define TIOCGWINSZ     0x5413   /* Get terminal window size (rows/cols) */
#define TIOCSWINSZ     0x5414   /* Set terminal window size */
#define TIOCMGET       0x5415   /* Get modem control line status (DTR, RTS, etc.) */
#define TIOCMBIS       0x5416   /* Set specified modem bits */
#define TIOCMBIC       0x5417   /* Clear specified modem bits */
#define TIOCMSET       0x5418   /* Set modem control line status */
#define TIOCGSOFTCAR   0x5419   /* Get soft carrier flag (ignore modem control lines) */
#define TIOCSSOFTCAR   0x541A   /* Set soft carrier flag */
#define FIONREAD       0x541B   /* Get number of bytes available to read */
#define TIOCINQ        FIONREAD /* Alias for FIONREAD */
#define TIOCLINUX      0x541C   /* Linux-specific operations (subcommand-based) */
#define TIOCCONS       0x541D   /* Redirect kernel console output to this tty */
#define TIOCGSERIAL    0x541E   /* Get serial port settings */
#define TIOCSSERIAL    0x541F   /* Set serial port settings */
#define TIOCPKT        0x5420   /* Enable/disable packet mode for ptys */
#define FIONBIO        0x5421   /* Set/clear non-blocking I/O mode */
#define TIOCNOTTY      0x5422   /* Disconnect controlling tty */
#define TIOCSETD       0x5423   /* Set line discipline */
#define TIOCGETD       0x5424   /* Get line discipline */
#define TCSBRKP        0x5425   /* Send break (POSIX tcsendbreak() support) */
#define TIOCTTYGSTRUCT 0x5426   /* Debug: get internal tty_struct pointer */
#define TIOCSBRK       0x5427   /* Set break condition (BSD-style) */
#define TIOCCBRK       0x5428   /* Clear break condition (BSD-style) */
#define TIOCGSID       0x5429   /* Get session ID of tty */
#define TIOCGPTN       _IOR('T', 0x30, unsigned int) /* Get pseudotty number (e.g. /dev/pts/N) */
#define TIOCSPTLCK     _IOW('T', 0x31, int)          /* Lock/unlock pseudotty to prevent reuse */

#endif                                               /* SYSCALL_IOCTL_H */
