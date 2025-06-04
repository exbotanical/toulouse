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

extern unsigned char ctype_t[];

#endif /* LIB_CTYPE_H */
