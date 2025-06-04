#ifndef DRIVER_DEV_CHAR_KEYMAP_H
#define DRIVER_DEV_CHAR_KEYMAP_H

#include "keyboard.h"

#define BS 127 /* Backspace */

typedef struct {
  unsigned char letter;
  unsigned char code;
} diacritic_t;

extern char *diacr_chars;
extern char *pad_chars;

extern unsigned short keymap[NUM_MODIFIERS * NUM_SCODES];

extern diacritic_t grave_table[NUM_DIACR];

extern diacritic_t acute_table[NUM_DIACR];

// Circumflex, Circumflex, Circumflex, vertical bar, vertical bar... :D
extern diacritic_t circm_table[NUM_DIACR];

// Diaeresis (umlaut)
extern diacritic_t diere_table[NUM_DIACR];

extern char *pad_seq[17];

extern char *fn_seq[20];

#endif /* DRIVER_DEV_CHAR_KEYMAP_H */
