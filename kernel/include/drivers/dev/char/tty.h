#ifndef DEV_TTY_H
#define DEV_TTY_H

#define NUM_TTYS 16

typedef struct {
  int placeholder;
} tty_t;

extern tty_t tty_table[];

void tty_init(void);

#endif /* DEV_TTY_H */
