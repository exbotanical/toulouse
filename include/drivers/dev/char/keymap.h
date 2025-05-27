#ifndef DRIVER_DEV_CHAR_KEYMAP_H
#define DRIVER_DEV_CHAR_KEYMAP_H

#include "keyboard.h"

#define BS 127 /* backspace */

typedef struct {
  unsigned char letter;
  unsigned char code;
} diacritic_t;

// clang-format off
unsigned short keymap[NUM_MODIFIERS * NUM_SCODES] = {
  /*
   * Standard US keyboard (default keymap) with 16 modifiers

   * ==================================================================================================================================================== */
  /*  00 - 0	*/	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /*  01 - ESC	*/	TAG_CTRL('['),	TAG_CTRL('['),	0,	0,	0,	0,	0,	0,	TAG_META('['),	0,	0,	0,	0,	0,	0,	0,
  /*  02 - 1	*/	'1',	'!',	0,	0,	0,	0,	0,	0,	TAG_META('1'),	0,	0,	0,	0,	0,	0,	0,
  /*  03 - 2	*/	'2',	'@',	'@',	0,	0,	0,	0,	0,	TAG_META('2'),	0,	0,	0,	0,	0,	0,	0,
  /*  04 - 3	*/	'3',	'#',	0,	0,	TAG_CTRL('['),	0,	0,	0,	TAG_META('3'),	0,	0,	0,	0,	0,	0,	0,
  /*  05 - 4	*/	'4',	'$',	'$',	0,	TAG_CTRL('\\'),0,	0,	0,	TAG_META('4'),	0,	0,	0,	0,	0,	0,	0,
  /*  06 - 5	*/	'5',	'%',	0,	0,	TAG_CTRL(']'),	0,	0,	0,	TAG_META('5'),	0,	0,	0,	0,	0,	0,	0,
  /*  07 - 6	*/	'6',	'^',	0,	0,	TAG_CTRL('^'),	0,	0,	0,	TAG_META('6'),	0,	0,	0,	0,	0,	0,	0,
  /*  08 - 7	*/	'7',	'&',	'{',	0,	TAG_CTRL('_'),	0,	0,	0,	TAG_META('7'),	0,	0,	0,	0,	0,	0,	0,
  /*  09 - 8	*/	'8',	'*',	'[',	0,	BS,	0,	0,	0,	TAG_META('8'),	0,	0,	0,	0,	0,	0,	0,
  /*  10 - 9	*/	'9',	'(',	']',	0,	0,	0,	0,	0,	TAG_META('9'),	0,	0,	0,	0,	0,	0,	0,
  /*  11 - 0	*/	'0',	')',	'}',	0,	0,	0,	0,	0,	TAG_META('0'),	0,	0,	0,	0,	0,	0,	0,
  /*  12 - -_	*/	'-',	'_',	'\\',	0,	TAG_CTRL('_'),	0,	0,	0,	TAG_META('-'),	0,	0,	0,	0,	0,	0,	0,
  /*  13 - =+	*/	'=',	'+',	0,	0,	0,	0,	0,	0,	TAG_META('='),	0,	0,	0,	0,	0,	0,	0,
  /*  14 - BS	*/	BS,	BS,	0,	0,	0,	0,	0,	0,	TAG_META(BS),	0,	0,	0,	0,	0,	0,	0,
  /*  15 - TAB	*/	'\t',	'\t',	0,	0,	0,	0,	0,	0,	TAG_META('\t'),0,	0,	0,	0,	0,	0,	0,
  /*  16 - q	*/	TAG_LETTER('q'),	TAG_LETTER('Q'),	TAG_LETTER('q'),	TAG_LETTER('Q'),	TAG_CTRL('Q'),	0,	0,	0,	TAG_META('q'),	0,	0,	0,	0,	0,	0,	0,
  /*  17 - w	*/	TAG_LETTER('w'),	TAG_LETTER('W'),	TAG_LETTER('w'),	TAG_LETTER('W'),	TAG_CTRL('W'),	0,	0,	0,	TAG_META('w'),	0,	0,	0,	0,	0,	0,	0,
  /*  18 - e	*/	TAG_LETTER('e'),	TAG_LETTER('E'),	TAG_LETTER('e'),	TAG_LETTER('E'),	TAG_CTRL('E'),	0,	0,	0,	TAG_META('e'),	0,	0,	0,	0,	0,	0,	0,
  /*  19 - r	*/	TAG_LETTER('r'),	TAG_LETTER('R'),	TAG_LETTER('r'),	TAG_LETTER('R'),	TAG_CTRL('R'),	0,	0,	0,	TAG_META('r'),	0,	0,	0,	0,	0,	0,	0,
  /*  20 - t	*/	TAG_LETTER('t'),	TAG_LETTER('T'),	TAG_LETTER('t'),	TAG_LETTER('T'),	TAG_CTRL('T'),	0,	0,	0,	TAG_META('t'),	0,	0,	0,	0,	0,	0,	0,
  /*  21 - y	*/	TAG_LETTER('y'),	TAG_LETTER('Y'),	TAG_LETTER('y'),	TAG_LETTER('Y'),	TAG_CTRL('Y'),	0,	0,	0,	TAG_META('y'),	0,	0,	0,	0,	0,	0,	0,
  /*  22 - u	*/	TAG_LETTER('u'),	TAG_LETTER('U'),	TAG_LETTER('u'),	TAG_LETTER('U'),	TAG_CTRL('U'),	0,	0,	0,	TAG_META('u'),	0,	0,	0,	0,	0,	0,	0,
  /*  23 - i	*/	TAG_LETTER('i'),	TAG_LETTER('I'),	TAG_LETTER('i'),	TAG_LETTER('I'),	TAG_CTRL('I'),	0,	0,	0,	TAG_META('i'),	0,	0,	0,	0,	0,	0,	0,
  /*  24 - o	*/	TAG_LETTER('o'),	TAG_LETTER('O'),	TAG_LETTER('o'),	TAG_LETTER('O'),	TAG_CTRL('O'),	0,	0,	0,	TAG_META('o'),	0,	0,	0,	0,	0,	0,	0,
  /*  25 - p	*/	TAG_LETTER('p'),	TAG_LETTER('P'),	TAG_LETTER('p'),	TAG_LETTER('P'),	TAG_CTRL('P'),	0,	0,	0,	TAG_META('p'),	0,	0,	0,	0,	0,	0,	0,
  /*  26 - [{	*/	'[',	'{',	0,	0,	TAG_CTRL('['),	0,	0,	0,	TAG_META('['),	0,	0,	0,	0,	0,	0,	0,
  /*  27 - ]}	*/	']',	'}',	'~',	0,	TAG_CTRL(']'),	0,	0,	0,	TAG_META(']'),	0,	0,	0,	0,	0,	0,	0,
  /*  28 - CR	*/	CR,	CR,	CR,	CR,	CR,	0,	0,	0,	TAG_META(CR),	0,	0,	0,	0,	0,	0,	0,
  /*  29 - LCTRL	*/	LCTRL,	LCTRL,	LCTRL,	LCTRL,	LCTRL,	0,	0,	0,	LCTRL,	0,	0,	0,	0,	0,	0,	0,
  /*  30 - a	*/	TAG_LETTER('a'),	TAG_LETTER('A'),	TAG_LETTER('a'),	TAG_LETTER('A'),	TAG_CTRL('A'),	0,	0,	0,	TAG_META('a'),	0,	0,	0,	0,	0,	0,	0,
  /*  31 - s	*/	TAG_LETTER('s'),	TAG_LETTER('S'),	TAG_LETTER('s'),	TAG_LETTER('S'),	TAG_CTRL('S'),	0,	0,	0,	TAG_META('s'),	0,	0,	0,	0,	0,	0,	0,
  /*  32 - d	*/	TAG_LETTER('d'),	TAG_LETTER('D'),	TAG_LETTER('d'),	TAG_LETTER('D'),	TAG_CTRL('D'),	0,	0,	0,	TAG_META('d'),	0,	0,	0,	0,	0,	0,	0,
  /*  33 - f	*/	TAG_LETTER('f'),	TAG_LETTER('F'),	TAG_LETTER('f'),	TAG_LETTER('F'),	TAG_CTRL('F'),	0,	0,	0,	TAG_META('f'),	0,	0,	0,	0,	0,	0,	0,
  /*  34 - g	*/	TAG_LETTER('g'),	TAG_LETTER('G'),	TAG_LETTER('g'),	TAG_LETTER('G'),	TAG_CTRL('G'),	0,	0,	0,	TAG_META('g'),	0,	0,	0,	0,	0,	0,	0,
  /*  35 - h	*/	TAG_LETTER('h'),	TAG_LETTER('H'),	TAG_LETTER('h'),	TAG_LETTER('H'),	TAG_CTRL('H'),	0,	0,	0,	TAG_META('h'),	0,	0,	0,	0,	0,	0,	0,
  /*  36 - j	*/	TAG_LETTER('j'),	TAG_LETTER('J'),	TAG_LETTER('j'),	TAG_LETTER('J'),	TAG_CTRL('J'),	0,	0,	0,	TAG_META('j'),	0,	0,	0,	0,	0,	0,	0,
  /*  37 - k	*/	TAG_LETTER('k'),	TAG_LETTER('K'),	TAG_LETTER('k'),	TAG_LETTER('K'),	TAG_CTRL('K'),	0,	0,	0,	TAG_META('k'),	0,	0,	0,	0,	0,	0,	0,
  /*  38 - l	*/	TAG_LETTER('l'),	TAG_LETTER('L'),	TAG_LETTER('l'),	TAG_LETTER('L'),	TAG_CTRL('L'),	0,	0,	0,	TAG_META('l'),	0,	0,	0,	0,	0,	0,	0,
  /*  39 - ;:	*/	';',	':',	0,	0,	0,	0,	0,	0,	TAG_META(';'),	0,	0,	0,	0,	0,	0,	0,
  /*  40 - '"	*/	'\'',	'"',	0,	0,	TAG_CTRL('G'),	0,	0,	0,	TAG_META('\''),0,	0,	0,	0,	0,	0,	0,
  /*  41 - `~	*/	'`',	'~',	0,	0,	0,	0,	0,	0,	TAG_META('`'),	0,	0,	0,	0,	0,	0,	0,
  /*  42 - LSHF	*/	LSHIFT,	LSHIFT,	LSHIFT,	LSHIFT,	LSHIFT,	0,	0,	0,	LSHIFT,	0,	0,	0,	0,	0,	0,	0,
  /*  43 - \|	*/	'\\',	'|',	0,	0,	TAG_CTRL('\\'),0,	0,	0,	TAG_META('\\'),0,	0,	0,	0,	0,	0,	0,
  /*  44 - z	*/	TAG_LETTER('z'),	TAG_LETTER('Z'),	TAG_LETTER('z'),	TAG_LETTER('Z'),	TAG_CTRL('Z'),	0,	0,	0,	TAG_META('z'),	0,	0,	0,	0,	0,	0,	0,
  /*  45 - x	*/	TAG_LETTER('x'),	TAG_LETTER('X'),	TAG_LETTER('x'),	TAG_LETTER('X'),	TAG_CTRL('X'),	0,	0,	0,	TAG_META('x'),	0,	0,	0,	0,	0,	0,	0,
  /*  46 - c	*/	TAG_LETTER('c'),	TAG_LETTER('C'),	TAG_LETTER('c'),	TAG_LETTER('C'),	TAG_CTRL('C'),	0,	0,	0,	TAG_META('c'),	0,	0,	0,	0,	0,	0,	0,
  /*  47 - v	*/	TAG_LETTER('v'),	TAG_LETTER('V'),	TAG_LETTER('v'),	TAG_LETTER('V'),	TAG_CTRL('V'),	0,	0,	0,	TAG_META('v'),	0,	0,	0,	0,	0,	0,	0,
  /*  48 - b	*/	TAG_LETTER('b'),	TAG_LETTER('B'),	TAG_LETTER('b'),	TAG_LETTER('B'),	TAG_CTRL('B'),	0,	0,	0,	TAG_META('b'),	0,	0,	0,	0,	0,	0,	0,
  /*  49 - n	*/	TAG_LETTER('n'),	TAG_LETTER('N'),	TAG_LETTER('n'),	TAG_LETTER('N'),	TAG_CTRL('N'),	0,	0,	0,	TAG_META('n'),	0,	0,	0,	0,	0,	0,	0,
  /*  50 - m	*/	TAG_LETTER('m'),	TAG_LETTER('M'),	TAG_LETTER('m'),	TAG_LETTER('M'),	TAG_CTRL('M'),	0,	0,	0,	TAG_META('m'),	0,	0,	0,	0,	0,	0,	0,
  /*  51 - ,<	*/	',',	'<',	0,	0,	0,	0,	0,	0,	TAG_META(','),	0,	0,	0,	0,	0,	0,	0,
  /*  52 - .>	*/	'.',	'>',	0,	0,	0,	0,	0,	0,	TAG_META('.'),	0,	0,	0,	0,	0,	0,	0,
  /*  53 - /?	*/	SLASH,	'?',	0,	0,	BS,	0,	0,	0,	TAG_META('/'),	0,	0,	0,	0,	0,	0,	0,
  /*  54 - RSHF	*/	RSHIFT,	RSHIFT,	RSHIFT,	RSHIFT,	RSHIFT,	0,	0,	0,	RSHIFT,	0,	0,	0,	0,	0,	0,	0,
  /*  55 - *	*/	ASTSK,	ASTSK,	ASTSK,	ASTSK,	ASTSK,	0,	0,	0,	ASTSK,	0,	0,	0,	0,	0,	0,	0,
  /*  56 - ALT	*/	ALT,	ALT,	ALT,	ALT,	ALT,	0,	0,	0,	ALT,	0,	0,	0,	0,	0,	0,	0,
  /*  57 - SPC	*/	' ',	' ',	0,	0,	0,	0,	0,	0,	TAG_META(' '),	0,	0,	0,	0,	0,	0,	0,
  /*  58 - CAPS	*/	CAPS,	CAPS,	CAPS,	CAPS,	CAPS,	0,	0,	0,	CAPS,	0,	0,	0,	0,	0,	0,	0,
  /*  59 - F1	*/	F1,	SF1,	0,	0,	F1,	0,	0,	0,	AF1,	0,	0,	0,	0,	0,	0,	0,
  /*  60 - F2	*/	F2,	SF2,	0,	0,	F2,	0,	0,	0,	AF2,	0,	0,	0,	0,	0,	0,	0,
  /*  61 - F3	*/	F3,	SF3,	0,	0,	F3,	0,	0,	0,	AF3,	0,	0,	0,	0,	0,	0,	0,
  /*  62 - F4	*/	F4,	SF4,	0,	0,	F4,	0,	0,	0,	AF4,	0,	0,	0,	0,	0,	0,	0,
  /*  63 - F5	*/	F5,	SF5,	0,	0,	F5,	0,	0,	0,	AF5,	0,	0,	0,	0,	0,	0,	0,
  /*  64 - F6	*/	F6,	SF6,	0,	0,	F6,	0,	0,	0,	AF6,	0,	0,	0,	0,	0,	0,	0,
  /*  65 - F7	*/	F7,	SF7,	0,	0,	F7,	0,	0,	0,	AF7,	0,	0,	0,	0,	0,	0,	0,
  /*  66 - F8	*/	F8,	SF8,	0,	0,	F8,	0,	0,	0,	AF8,	0,	0,	0,	0,	0,	0,	0,
  /*  67 - F9	*/	F9,	SF9,	0,	0,	F9,	0,	0,	0,	AF9,	0,	0,	0,	0,	0,	0,	0,
  /*  68 - F10	*/	F10,	SF10,	0,	0,	F10,	0,	0,	0,	AF10,	0,	0,	0,	0,	0,	0,	0,
  /*  69 - NUMS	*/	NUMS,	NUMS,	NUMS,	NUMS,	NUMS,	0,	0,	0,	NUMS,	0,	0,	0,	0,	0,	0,	0,
  /*  70 - SCRL	*/	SCRL,	SCRL3,	SCRL2,	0,	SCRL4,	0,	0,	0,	SCRL,	0,	0,	0,	0,	0,	0,	0,
  /*  71 - HOME/7	*/	HOME,	HOME,	HOME,	HOME,	HOME,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /*  72 - UP  /8	*/	UP,	UP,	UP,	UP,	UP,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /*  73 - PGUP/9	*/	PGUP,	PGUP,	PGUP,	PGUP,	PGUP,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /*  74 - MINUS	*/	MINUS,	MINUS,	MINUS,	MINUS,	MINUS,	0,	0,	0,	MINUS,	0,	0,	0,	0,	0,	0,	0,
  /*  75 - LEFT/4	*/	LEFT,	LEFT,	LEFT,	LEFT,	LEFT,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /*  76 - MID /5	*/	MID,	MID,	MID,	MID,	MID,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /*  77 - RIGH/6	*/	RIGHT,	RIGHT,	RIGHT,	RIGHT,	RIGHT,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /*  78 - PLUS	*/	PLUS,	PLUS,	PLUS,	PLUS,	PLUS,	0,	0,	0,	PLUS,	0,	0,	0,	0,	0,	0,	0,
  /*  79 - END /1	*/	END,	END,	END,	END,	END,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /*  80 - DOWN/2	*/	DOWN,	DOWN,	DOWN,	DOWN,	DOWN,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /*  81 - PGDN/3	*/	PGDN,	PGDN,	PGDN,	PGDN,	PGDN,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /*  82 - INS /0	*/	INS,	INS,	INS,	INS,	INS,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /*  83 - DEL /.	*/	DEL,	DEL,	DEL,	DEL,	DEL,	0,	0,	0,	DEL,	0,	0,	0,	0,	0,	0,	0,
  /*  84 - SYSRQ	*/	SYSRQ,	0,	0,	0,	0,	0,	0,	0,	SYSRQ,	0,	0,	0,	0,	0,	0,	0,
  /*  85 -	*/	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /*  86 - <>	*/	'<',	'>',	'|',	0,	0,	0,	0,	0,	TAG_META('<'),	0,	0,	0,	0,	0,	0,	0,
  /*  87 - F11	*/	SF1,	SF1,	0,	0,	F11,	0,	0,	0,	AF11,	0,	0,	0,	0,	0,	0,	0,
  /*  88 - F12	*/	SF2,	SF2,	0,	0,	F12,	0,	0,	0,	AF12,	0,	0,	0,	0,	0,	0,	0,
  /*  89 -	*/	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /*  90 -	*/	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /*  91 -	*/	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /*  92 -	*/	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /*  93 -	*/	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /*  94 -	*/	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /*  95 -	*/	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /*  96 - E0ENTER*/	E0ENTER,E0ENTER,E0ENTER,E0ENTER,E0ENTER,0,	0,	0,	E0ENTER,0,	0,	0,	0,	0,	0,	0,
  /*  97 - RCTRL	*/	RCTRL,	RCTRL,	RCTRL,	RCTRL,	RCTRL,	0,	0,	0,	RCTRL,	0,	0,	0,	0,	0,	0,	0,
  /*  98 - E0SLASH*/	E0SLASH,E0SLASH,E0SLASH,E0SLASH,E0SLASH,0,	0,	0,	E0SLASH,0,	0,	0,	0,	0,	0,	0,
  /*  99 -	*/	0,	0,	0,	0,	TAG_CTRL('\\'),0,	0,	0,	TAG_CTRL('\\'),0,	0,	0,	0,	0,	0,	0,
  /* 100 - ALTGR	*/	ALTGR,	ALTGR,	ALTGR,	ALTGR,	ALTGR,	0,	0,	0,	ALTGR,	0,	0,	0,	0,	0,	0,	0,
  /* 101 -	*/	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /* 102 - E0HOME	*/	E0HOME,	E0HOME,	E0HOME,	E0HOME,	E0HOME,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /* 103 - E0UP  	*/	E0UP,	E0UP,	E0UP,	E0UP,	E0UP,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /* 104 - E0PGUP	*/	E0PGUP,	E0PGUP,	E0PGUP,	E0PGUP,	E0PGUP,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /* 105 - E0LEFT	*/	E0LEFT,	E0LEFT,	E0LEFT,	E0LEFT,	E0LEFT,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /* 106 - E0RIGHT*/	E0RIGHT,E0RIGHT,E0RIGHT,E0RIGHT,E0RIGHT,0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /* 107 - E0END	*/	E0END,	E0END,	E0END,	E0END,	E0END,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /* 108 - E0DOWN	*/	E0DOWN,	E0DOWN,	E0DOWN,	E0DOWN,	E0DOWN,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /* 109 - E0PGDN	*/	E0PGDN,	E0PGDN,	E0PGDN,	E0PGDN,	E0PGDN,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /* 110 - E0INS	*/	E0INS,	E0INS,	E0INS,	E0INS,	E0INS,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /* 111 - E0DEL	*/	E0DEL,	E0DEL,	E0DEL,	E0DEL,	E0DEL,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /* 112 -	*/	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /* 113 -	*/	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /* 114 -	*/	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /* 115 -	*/	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /* 116 -	*/	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /* 117 -	*/	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /* 118 -	*/	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /* 119 -	*/	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /* 120 -	*/	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /* 121 -	*/	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /* 122 -	*/	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /* 123 -	*/	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /* 124 -	*/	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /* 125 -	*/	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /* 126 -	*/	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
  /* 127 -	*/	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
};

static char *diacr_chars = "`'^ \"";
diacritic_t grave_table[NR_DIACR] = {
	{ 'A', '\300' },
	{ 'E', '\310' },
	{ 'I', '\314' },
	{ 'O', '\322' },
	{ 'U', '\331' },
	{ 'a', '\340' },
	{ 'e', '\350' },
	{ 'i', '\354' },
	{ 'o', '\362' },
	{ 'u', '\371' },
};
diacritic_t acute_table[NR_DIACR] = {
	{ 'A', '\301' },
	{ 'E', '\311' },
	{ 'I', '\315' },
	{ 'O', '\323' },
	{ 'U', '\332' },
	{ 'a', '\341' },
	{ 'e', '\351' },
	{ 'i', '\355' },
	{ 'o', '\363' },
	{ 'u', '\372' },
};
// Circumflex, Circumflex, Circumflex, vertical bar, vertical bar... :D
diacritic_t circm_table[NR_DIACR] = {
	{ 'A', '\302' },
	{ 'E', '\312' },
	{ 'I', '\316' },
	{ 'O', '\324' },
	{ 'U', '\333' },
	{ 'a', '\342' },
	{ 'e', '\352' },
	{ 'i', '\356' },
	{ 'o', '\364' },
	{ 'u', '\373' },
};
// Diaeresis (umlaut)
diacritic_t diere_table[NR_DIACR] = {
	{ 'A', '\304' },
	{ 'E', '\313' },
	{ 'I', '\317' },
	{ 'O', '\326' },
	{ 'U', '\334' },
	{ 'a', '\344' },
	{ 'e', '\353' },
	{ 'i', '\357' },
	{ 'o', '\366' },
	{ 'u', '\374' },
};

static char *pad_chars = "0123456789+-*/\015,.";

static char *pad_seq[] = {
	"\033[2~",	/* INS */
	"\033[4~",	/* END */
	"\033[B" ,	/* DOWN */
	"\033[6~",	/* PGDN */
	"\033[D" ,	/* LEFT */
	"\033[G" ,	/* MID */
	"\033[C" ,	/* RIGHT */
	"\033[1~",	/* HOME */
	"\033[A" ,	/* UP */
	"\033[5~",	/* PGUP */
	"+",		/* PLUS */
	"-",		/* MINUS */
	"*",		/* ASTERISK */
	"/",		/* SLASH */
	"'\n'",		/* ENTER */
	",",		/* COMMA */
	"\033[3~",	/* DEL */
};

static char *fn_seq[] = {
	"\033[[A",	/* F1 */
	"\033[[B",	/* F2 */
	"\033[[C",	/* F3 */
	"\033[[D",	/* F4 */
	"\033[[E",	/* F5 */
	"\033[17~",	/* F6 */
	"\033[18~",	/* F7 */
	"\033[19~",	/* F8 */
	"\033[20~",	/* F9 */
	"\033[21~",	/* F10 */
	"\033[23~",	/* F11, SF1 */
	"\033[24~",	/* F12, SF2 */
	"\033[25~",	/* SF3 */
	"\033[26~",	/* SF4 */
	"\033[28~",	/* SF5 */
	"\033[29~",	/* SF6 */
	"\033[31~",	/* SF7 */
	"\033[32~",	/* SF8 */
	"\033[33~",	/* SF9 */
	"\033[34~",	/* SF10 */
};
// clang-format on

#endif /* DRIVER_DEV_CHAR_KEYMAP_H */
