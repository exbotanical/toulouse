#ifndef TTY_H
#define TTY_H

#define NUM_TTYS 16

typedef struct {
  int placeholder;
} tty_t;

extern tty_t tty_table[];

#endif /* TTY_H */
