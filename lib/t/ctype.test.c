#include "lib/ctype.h"

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "lib/ctype.h"
#include "tests.h"

static void
isalpha_test (void) {
  ok(ISALPHA('A'), "ISALPHA: 'A' is alphabetic");
  ok(ISALPHA('z'), "ISALPHA: 'z' is alphabetic");
  ok(!ISALPHA('1'), "ISALPHA: '1' is not alphabetic");
  ok(!ISALPHA('@'), "ISALPHA: '@' is not alphabetic");
  ok(!ISALPHA(0x7F), "ISALPHA: DEL is not alphabetic");
  ok(!ISALPHA('\n'), "ISALPHA: newline is not alphabetic");
}

static void
isupper_test (void) {
  ok(ISUPPER('A'), "ISUPPER: 'A' is uppercase");
  ok(ISUPPER('Z'), "ISUPPER: 'Z' is uppercase");
  ok(!ISUPPER('a'), "ISUPPER: 'a' is not uppercase");
  ok(!ISUPPER('0'), "ISUPPER: '0' is not uppercase");
}

static void
islower_test (void) {
  ok(ISLOWER('a'), "ISLOWER: 'a' is lowercase");
  ok(ISLOWER('z'), "ISLOWER: 'z' is lowercase");
  ok(!ISLOWER('A'), "ISLOWER: 'A' is not lowercase");
  ok(!ISLOWER('9'), "ISLOWER: '9' is not lowercase");
}

static void
isdigit_test (void) {
  ok(ISDIGIT('0'), "ISDIGIT: '0' is digit");
  ok(ISDIGIT('9'), "ISDIGIT: '9' is digit");
  ok(!ISDIGIT('a'), "ISDIGIT: 'a' is not digit");
  ok(!ISDIGIT('Z'), "ISDIGIT: 'Z' is not digit");
}

static void
isalnum_test (void) {
  ok(ISALNUM('0'), "ISALNUM: '0' is alphanumeric");
  ok(ISALNUM('A'), "ISALNUM: 'A' is alphanumeric");
  ok(ISALNUM('z'), "ISALNUM: 'z' is alphanumeric");
  ok(!ISALNUM('@'), "ISALNUM: '@' is not alphanumeric");
  ok(!ISALNUM('\t'), "ISALNUM: tab is not alphanumeric");
}

static void
isspace_test (void) {
  ok(ISSPACE(' '), "ISSPACE: space is spacing");
  ok(ISSPACE('\t'), "ISSPACE: tab is spacing");
  ok(ISSPACE('\n'), "ISSPACE: newline is spacing");
  ok(ISSPACE('\f'), "ISSPACE: formfeed is spacing");
  ok(ISSPACE('\v'), "ISSPACE: vertical tab is spacing");
  ok(!ISSPACE('A'), "ISSPACE: 'A' is not spacing");
}

static void
ispunct_test (void) {
  ok(ISPUNCT('!'), "ISPUNCT: '!' is punctuation");
  ok(ISPUNCT(','), "ISPUNCT: ',' is punctuation");
  ok(ISPUNCT('~'), "ISPUNCT: '~' is punctuation");
  ok(!ISPUNCT('a'), "ISPUNCT: 'a' is not punctuation");
}

static void
iscntrl_test (void) {
  ok(ISCTRL('\0'), "ISCTRL: NULL is control");
  ok(ISCTRL(0x1F), "ISCTRL: 0x1F is control");
  ok(ISCTRL(0x7F), "ISCTRL: DEL is control");
  ok(!ISCTRL('A'), "ISCTRL: 'A' is not control");
  ok(!ISCTRL(' '), "ISCTRL: space is not control");
}

static void
isxdigit_test (void) {
  ok(ISXDIGIT('0'), "ISXDIGIT: '0' is hex digit");
  ok(ISXDIGIT('9'), "ISXDIGIT: '9' is hex digit");
  ok(ISXDIGIT('a'), "ISXDIGIT: 'a' is hex digit");
  ok(ISXDIGIT('F'), "ISXDIGIT: 'F' is hex digit");
  ok(!ISXDIGIT('g'), "ISXDIGIT: 'g' is not hex digit");
  ok(!ISXDIGIT('@'), "ISXDIGIT: '@' is not hex digit");
}

static void
isascii_test (void) {
  ok(ISASCII(0), "ISASCII: 0 is ASCII");
  ok(ISASCII(127), "ISASCII: 127 is ASCII");
  ok(!ISASCII(128), "ISASCII: 128 is not ASCII");
  ok(!ISASCII(255), "ISASCII: 255 is not ASCII");
}

static void
toascii_test (void) {
  eq_num(TOASCII(0xFF), 0x7F, "TOASCII: 0xFF becomes 0x7F");
  eq_num(TOASCII(0x80), 0x00, "TOASCII: 0x80 becomes 0x00");
}

static void
toupper_test (void) {
  eq_num(TOUPPER('a'), 'A', "TOUPPER: 'a' becomes 'A'");
  eq_num(TOUPPER('z'), 'Z', "TOUPPER: 'z' becomes 'Z'");
  eq_num(TOUPPER('A'), 'A', "TOUPPER: 'A' stays 'A'");
}

static void
tolower_test (void) {
  eq_num(TOLOWER('A'), 'a', "TOLOWER: 'A' becomes 'a'");
  eq_num(TOLOWER('Z'), 'z', "TOLOWER: 'Z' becomes 'z'");
  eq_num(TOLOWER('z'), 'z', "TOLOWER: 'z' stays 'z'");
}

void
run_ctype_tests (void) {
  isalpha_test();
  isupper_test();
  islower_test();
  isdigit_test();
  isalnum_test();
  isspace_test();
  ispunct_test();
  iscntrl_test();
  isxdigit_test();
  isascii_test();
  toascii_test();
  toupper_test();
  tolower_test();
}
