#ifndef DRIVER_DEV_CHAR_KEYBOARD_H
#define DRIVER_DEV_CHAR_KEYBOARD_H

#define KEYBOARD_IRQ  1

#define EXTKEY        0xE0               /* extended key (AltGr, Ctrl-Print, etc.) */

#define NUM_MODIFIERS 16                 /* Max. number of modifiers per keymap */
#define NUM_SCODES    128                /* Max. number of scancodes */
#define NUM_DIACR     10

#define FN_KEYS       0x100              /* Function keys (F1–F12, etc.) */
#define SPEC_KEYS     0x200              /* Special keys (Enter, Esc, Tab) */
#define PAD_KEYS      0x300              /* Numeric keypad keys */
#define DEAD_KEYS     0x400              /* Dead keys (e.g. accents) */
#define CONS_KEYS     0x500              /* Console switching (ALT+Fn) */
#define SHIFT_KEYS    0x700              /* Modifier keys (Shift, Ctrl, Alt) */
#define META_KEYS     0x800              /* Meta keys (used with A(c)) */
#define LETTER_KEYS   0xB00              /* Affected by Caps Lock */

#define CR            (0x01 + SPEC_KEYS) /* Carriage return */
#define SLASH_NPAD    53

/**
 * Scroll lock - used for SysRq features trigger debug/display functions when Scroll Lock is used as
 * a modifier
 */
#define SCRL2         (0x02 + SPEC_KEYS) /* SH_REGS (show registers) */
#define SCRL3         (0x03 + SPEC_KEYS) /* SH_MEM (show memory) */
#define SCRL4         (0x04 + SPEC_KEYS) /* SH_STAT (show status) */
#define SYSRQ         (0x06 + SPEC_KEYS) /* Signals entry into SysRq mode */
#define CAPS          (0x07 + SPEC_KEYS) /* Caps lock */
#define NUMS          (0x08 + SPEC_KEYS) /* Num lock */
#define SCRL          (0x09 + SPEC_KEYS) /* Scroll lock */

#define SHIFT         (0x00 + SHIFT_KEYS)
#define CTRL          (0x02 + SHIFT_KEYS)
#define ALT           (0x03 + SHIFT_KEYS)
#define LSHIFT        (0x04 + SHIFT_KEYS)
#define RSHIFT        (0x05 + SHIFT_KEYS)
#define LCTRL         (0x06 + SHIFT_KEYS)

#define INS           (0x00 + PAD_KEYS)
#define END           (0x01 + PAD_KEYS)
#define DOWN          (0x02 + PAD_KEYS)
#define PGDN          (0x03 + PAD_KEYS)
#define LEFT          (0x04 + PAD_KEYS)
#define MID           (0x05 + PAD_KEYS)
#define RIGHT         (0x06 + PAD_KEYS)
#define HOME          (0x07 + PAD_KEYS)
#define UP            (0x08 + PAD_KEYS)
#define PGUP          (0x09 + PAD_KEYS)
#define PLUS          (0x0A + PAD_KEYS)
#define MINUS         (0x0B + PAD_KEYS)
#define ASTSK         (0x0C + PAD_KEYS)
#define SLASH         (0x0D + PAD_KEYS)
#define ENTER         (0x0E + PAD_KEYS)
#define DEL           (0x10 + PAD_KEYS)

/* Function keys */
#define F1            (0x00 + FN_KEYS)
#define F2            (0x01 + FN_KEYS)
#define F3            (0x02 + FN_KEYS)
#define F4            (0x03 + FN_KEYS)
#define F5            (0x04 + FN_KEYS)
#define F6            (0x05 + FN_KEYS)
#define F7            (0x06 + FN_KEYS)
#define F8            (0x07 + FN_KEYS)
#define F9            (0x08 + FN_KEYS)
#define F10           (0x09 + FN_KEYS)
#define F11           (0x0A + FN_KEYS)
#define F12           (0x0B + FN_KEYS)

/* Shifted function keys */
#define SF1           (0x0A + FN_KEYS)
#define SF2           (0x0B + FN_KEYS)
#define SF3           (0x0C + FN_KEYS)
#define SF4           (0x0D + FN_KEYS)
#define SF5           (0x0E + FN_KEYS)
#define SF6           (0x0F + FN_KEYS)
#define SF7           (0x10 + FN_KEYS)
#define SF8           (0x11 + FN_KEYS)
#define SF9           (0x12 + FN_KEYS)
#define SF10          (0x13 + FN_KEYS)
#define SF11          (0x0A + SHIFT)
#define SF12          (0x0B + SHIFT)

/* Alt function keys */
#define AF1           (0x00 + CONS_KEYS)
#define AF2           (0x01 + CONS_KEYS)
#define AF3           (0x02 + CONS_KEYS)
#define AF4           (0x03 + CONS_KEYS)
#define AF5           (0x04 + CONS_KEYS)
#define AF6           (0x05 + CONS_KEYS)
#define AF7           (0x06 + CONS_KEYS)
#define AF8           (0x07 + CONS_KEYS)
#define AF9           (0x08 + CONS_KEYS)
#define AF10          (0x09 + CONS_KEYS)
#define AF11          (0x0A + CONS_KEYS)
#define AF12          (0x0B + CONS_KEYS)

#define GRAVE         (0x00 + DEAD_KEYS)
#define ACUTE         (0x01 + DEAD_KEYS)
#define CIRCM         (0x02 + DEAD_KEYS)
#define DIERE         (0x04 + DEAD_KEYS)

/**
 * E0 keys. E0 refers to a special prefix byte (0xE0) in the PS/2 keyboard scancode set.
 * This prefix indicates that the key is an extended key — usually from the numeric keypad,
 * navigation cluster (Home/End/etc), right-hand modifiers (Right Ctrl/AltGr), or multimedia keys.
 *
 * Most keys send a single-byte scancode. Extended keys send two bytes: 0xE0 followed by another
 * byte. Some keys like Left Ctrl and Right Ctrl use the same scancode except for the E0 prefix. The
 * prefix distinguishes between physically different keys.
 */
#define E0ENTER       96
#define RCTRL         97
#define E0SLASH       98
#define ALTGR         100 /* Alt graphic */
#define E0HOME        102
#define E0UP          103
#define E0PGUP        104
#define E0LEFT        105
#define E0RIGHT       106
#define E0END         107
#define E0DOWN        108
#define E0PGDN        109
#define E0INS         110
#define E0DEL         111

#define MOD_BASE      0
#define MOD_SHIFT     1
#define MOD_ALTGR     2
#define MOD_CTRL      3
#define MOD_ALT       4
#define MOD_SHIFTL    5
#define MOD_SHIFTR    6
#define MOD_CTRLL     7

#define LED_SCRLBIT   0x01         /* scroll lock led */
#define LED_NUMSBIT   0x02         /* num lock led */
#define LED_CAPSBIT   0x04         /* caps lock led */

#define TAG_CTRL(c)   ((c) & 0x1F) /* Converts key to ctrl code e.g. 'C' & 0x1F = 0x03 = Ctrl-C */
#define TAG_META(c)   ((c) | META_KEYS)   /* Tags key as meta key e.g. alt+key */
#define TAG_LETTER(c) ((c) | LETTER_KEYS) /* Tags key as printable letter */

#endif                                    /* DRIVER_DEV_CHAR_KEYBOARD_H */
