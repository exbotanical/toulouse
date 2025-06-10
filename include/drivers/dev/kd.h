#ifndef DRIVERS_DEV_KD_H
#define DRIVERS_DEV_KD_H

/* Keyboard/Display -> KD */

#define KD_TEXT     0x00
#define KD_GRAPHICS 0x01
#define KD_TEXT0    0x02 /* obsolete */
#define KD_TEXT1    0x03 /* obsolete */

typedef struct {
  unsigned char      kb_table;
  unsigned char      kb_index;
  unsigned short int kb_value;
} kbentry_t;

#endif /* DRIVERS_DEV_KD_H */
