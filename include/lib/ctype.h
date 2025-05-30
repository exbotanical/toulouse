#ifndef LIB_CTYPE_H
#define LIB_CTYPE_H

#define CTYPE_FUPPER 0x01 /* upper case */
#define CTYPE_FLOWER 0x02 /* lower case */
#define CTYPE_FNUMRL 0x04 /* numeral (digit) */
#define CTYPE_FSPC   0x08 /* spacing character */
#define CTYPE_FPUNC  0x10 /* punctuation */
#define CTYPE_FCTRL  0x20 /* control character */
#define CTYPE_FHEX   0x40 /* hexadecimal */
#define CTYPE_FBLNK  0x80 /* blank */

#define ISALPHA(ch)  ((ctype_t + 1)[ch] & (CTYPE_FUPPER | CTYPE_FLOWER))
#define ISUPPER(ch)  ((ctype_t + 1)[ch] & CTYPE_FUPPER)
#define ISLOWER(ch)  ((ctype_t + 1)[ch] & CTYPE_FLOWER)
#define ISDIGIT(ch)  ((ctype_t + 1)[ch] & CTYPE_FNUMRL)
#define ISALNUM(ch)  ((ctype_t + 1)[ch] & (CTYPE_FUPPER | CTYPE_FLOWER | CTYPE_FNUMRL))
#define ISSPACE(ch)  ((ctype_t + 1)[ch] & CTYPE_FSPC)
#define ISPUNCT(ch)  ((ctype_t + 1)[ch] & CTYPE_FPUNC)
#define ISCTRL(ch)   ((ctype_t + 1)[ch] & CTYPE_FCTRL)
#define ISXDIGIT(ch) ((ctype_t + 1)[ch] & (CTYPE_FNUMRL | CTYPE_FHEX))

#define ISASCII(ch)  ((unsigned)ch <= 0x7F)
#define TOASCII(ch)  ((unsigned)ch & 0x7F)

#define TOUPPER(ch)  ((ch) & ~32)
#define TOLOWER(ch)  ((ch) | 32)

unsigned char ctype_t[] = {
  0,
  CTYPE_FCTRL,               /* ^@   0x00 (NUL '\0') */
  CTYPE_FCTRL,               /* ^A   0x01 (SOH) */
  CTYPE_FCTRL,               /* ^B   0x02 (STX) */
  CTYPE_FCTRL,               /* ^C   0x03 (ETX) */
  CTYPE_FCTRL,               /* ^D   0x04 (EOT) */
  CTYPE_FCTRL,               /* ^E   0x05 (ENQ) */
  CTYPE_FCTRL,               /* ^F   0x06 (ACK) */
  CTYPE_FCTRL,               /* ^G   0x07 (BEL '\a') */
  CTYPE_FCTRL,               /* ^H   0x08 (BS  '\b') */
  CTYPE_FCTRL | CTYPE_FSPC,  /* ^I   0x09 (HT  '\t') */
  CTYPE_FCTRL | CTYPE_FSPC,  /* ^J   0x0A (LF  '\n') */
  CTYPE_FCTRL | CTYPE_FSPC,  /* ^K   0x0B (VT  '\v') */
  CTYPE_FCTRL | CTYPE_FSPC,  /* ^L   0x0C (FF  '\f') */
  CTYPE_FCTRL | CTYPE_FSPC,  /* ^M   0x0D (CR  '\r') */
  CTYPE_FCTRL,               /* ^N   0x0E (SO) */
  CTYPE_FCTRL,               /* ^O   0x0F (SI) */
  CTYPE_FCTRL,               /* ^P   0x10 (DLE) */
  CTYPE_FCTRL,               /* ^Q   0x11 (DC1) */
  CTYPE_FCTRL,               /* ^R   0x12 (DC2) */
  CTYPE_FCTRL,               /* ^S   0x13 (DC3) */
  CTYPE_FCTRL,               /* ^T   0x14 (DC4) */
  CTYPE_FCTRL,               /* ^U   0x15 (NAK) */
  CTYPE_FCTRL,               /* ^V   0x16 (SYN) */
  CTYPE_FCTRL,               /* ^W   0x17 (ETB) */
  CTYPE_FCTRL,               /* ^X   0x18 (CAN) */
  CTYPE_FCTRL,               /* ^Y   0x19 (EM) */
  CTYPE_FCTRL,               /* ^Z   0x1A (SUB) */
  CTYPE_FCTRL,               /* ^[   0x1B (ESC) */
  CTYPE_FCTRL,               /* ^\   0x1C (FS) */
  CTYPE_FCTRL,               /* ^]   0x1D (GS) */
  CTYPE_FCTRL,               /* ^^   0x1E (RS) */
  CTYPE_FCTRL,               /* ^_   0x1F (US) */
  CTYPE_FSPC,                /* ' '  0x20 */
  CTYPE_FPUNC,               /* '!'  0x21 */
  CTYPE_FPUNC,               /* '"'  0x22 */
  CTYPE_FPUNC,               /* '#'  0x23 */
  CTYPE_FPUNC,               /* '$'  0x24 */
  CTYPE_FPUNC,               /* '%'  0x25 */
  CTYPE_FPUNC,               /* '&'  0x26 */
  CTYPE_FPUNC,               /* '''  0x27 */
  CTYPE_FPUNC,               /* '('  0x28 */
  CTYPE_FPUNC,               /* ')'  0x29 */
  CTYPE_FPUNC,               /* '*'  0x2A */
  CTYPE_FPUNC,               /* '+'  0x2B */
  CTYPE_FPUNC,               /* ','  0x2C */
  CTYPE_FPUNC,               /* '-'  0x2D */
  CTYPE_FPUNC,               /* '.'  0x2E */
  CTYPE_FPUNC,               /* '/'  0x2F */
  CTYPE_FNUMRL,              /* '0'  0x30 */
  CTYPE_FNUMRL,              /* '1'  0x31 */
  CTYPE_FNUMRL,              /* '2'  0x32 */
  CTYPE_FNUMRL,              /* '3'  0x33 */
  CTYPE_FNUMRL,              /* '4'  0x34 */
  CTYPE_FNUMRL,              /* '5'  0x35 */
  CTYPE_FNUMRL,              /* '6'  0x36 */
  CTYPE_FNUMRL,              /* '7'  0x37 */
  CTYPE_FNUMRL,              /* '8'  0x38 */
  CTYPE_FNUMRL,              /* '9'  0x39 */
  CTYPE_FPUNC,               /* ':'  0x3A */
  CTYPE_FPUNC,               /* ';'  0x3B */
  CTYPE_FPUNC,               /* '<'  0x3C */
  CTYPE_FPUNC,               /* '='  0x3D */
  CTYPE_FPUNC,               /* '>'  0x3E */
  CTYPE_FPUNC,               /* '?'  0x3F */
  CTYPE_FPUNC,               /* '@'  0x40 */
  CTYPE_FUPPER | CTYPE_FHEX, /* 'A'  0x41 */
  CTYPE_FUPPER | CTYPE_FHEX, /* 'B'  0x42 */
  CTYPE_FUPPER | CTYPE_FHEX, /* 'C'  0x43 */
  CTYPE_FUPPER | CTYPE_FHEX, /* 'D'  0x44 */
  CTYPE_FUPPER | CTYPE_FHEX, /* 'E'  0x45 */
  CTYPE_FUPPER | CTYPE_FHEX, /* 'F'  0x46 */
  CTYPE_FUPPER,              /* 'G'  0x47 */
  CTYPE_FUPPER,              /* 'H'  0x48 */
  CTYPE_FUPPER,              /* 'I'  0x49 */
  CTYPE_FUPPER,              /* 'J'  0x4A */
  CTYPE_FUPPER,              /* 'K'  0x4B */
  CTYPE_FUPPER,              /* 'L'  0x4C */
  CTYPE_FUPPER,              /* 'M'  0x4D */
  CTYPE_FUPPER,              /* 'N'  0x4E */
  CTYPE_FUPPER,              /* 'O'  0x4F */
  CTYPE_FUPPER,              /* 'P'  0x50 */
  CTYPE_FUPPER,              /* 'Q'  0x51 */
  CTYPE_FUPPER,              /* 'R'  0x52 */
  CTYPE_FUPPER,              /* 'S'  0x53 */
  CTYPE_FUPPER,              /* 'T'  0x54 */
  CTYPE_FUPPER,              /* 'U'  0x55 */
  CTYPE_FUPPER,              /* 'V'  0x56 */
  CTYPE_FUPPER,              /* 'W'  0x57 */
  CTYPE_FUPPER,              /* 'X'  0x58 */
  CTYPE_FUPPER,              /* 'Y'  0x59 */
  CTYPE_FUPPER,              /* 'Z'  0x5A */
  CTYPE_FPUNC,               /* '['  0x5B */
  CTYPE_FPUNC,               /* '\'  0x5C */
  CTYPE_FPUNC,               /* ']'  0x5D */
  CTYPE_FPUNC,               /* '^'  0x5E */
  CTYPE_FPUNC,               /* '_'  0x5F */
  CTYPE_FPUNC,               /* '`'  0x60 */
  CTYPE_FLOWER | CTYPE_FHEX, /* 'a'  0x61 */
  CTYPE_FLOWER | CTYPE_FHEX, /* 'b'  0x62 */
  CTYPE_FLOWER | CTYPE_FHEX, /* 'c'  0x63 */
  CTYPE_FLOWER | CTYPE_FHEX, /* 'd'  0x64 */
  CTYPE_FLOWER | CTYPE_FHEX, /* 'e'  0x65 */
  CTYPE_FLOWER | CTYPE_FHEX, /* 'f'  0x66 */
  CTYPE_FLOWER,              /* 'g'  0x67 */
  CTYPE_FLOWER,              /* 'h'  0x68 */
  CTYPE_FLOWER,              /* 'i'  0x69 */
  CTYPE_FLOWER,              /* 'j'  0x6A */
  CTYPE_FLOWER,              /* 'k'  0x6B */
  CTYPE_FLOWER,              /* 'l'  0x6C */
  CTYPE_FLOWER,              /* 'm'  0x6D */
  CTYPE_FLOWER,              /* 'n'  0x6E */
  CTYPE_FLOWER,              /* 'o'  0x6F */
  CTYPE_FLOWER,              /* 'p'  0x70 */
  CTYPE_FLOWER,              /* 'q'  0x71 */
  CTYPE_FLOWER,              /* 'r'  0x72 */
  CTYPE_FLOWER,              /* 's'  0x73 */
  CTYPE_FLOWER,              /* 't'  0x74 */
  CTYPE_FLOWER,              /* 'u'  0x75 */
  CTYPE_FLOWER,              /* 'v'  0x76 */
  CTYPE_FLOWER,              /* 'w'  0x77 */
  CTYPE_FLOWER,              /* 'x'  0x78 */
  CTYPE_FLOWER,              /* 'y'  0x79 */
  CTYPE_FLOWER,              /* 'z'  0x7A */
  CTYPE_FPUNC,               /* '{'  0x7B */
  CTYPE_FPUNC,               /* '|'  0x7C */
  CTYPE_FPUNC,               /* '}'  0x7D */
  CTYPE_FPUNC,               /* '~'  0x7E */
  CTYPE_FCTRL,               /* DEL  0x7F */
};

#endif                       /* LIB_CTYPE_H */
