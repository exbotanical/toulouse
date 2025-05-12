#include "lib/string.h"

#include <stdio.h>

#include "tests.h"

void
kstrlen_test (void) {
  char* s = "katenv binasu\0";
  ok(kstrlen(s) == 13, "returns the number of bytes in the string pointed to by s");
}

void
kstrlen_test_empty (void) {
  char* s = "\0";
  ok(kstrlen(s) == 0, "returns zero for empty string (and ignores terminating null byte)");
}

void
kstrchr_test (void) {
  char* og  = "helelo";
  char* ret = kstrchr(og, 'e');

  is(ret, "elelo", "returns a char pointer starting at the first instance of the specified char");
  is(og, "helelo", "does not modify the original char pointer");
  *ret++;
  is(ret, "lelo", "returns a new char pointer");
}

void
kstrchr_test_null_byte (void) {
  char* ret = kstrchr("postmodernism sux", '\0');
  is(ret, "", "returns a pointer to the null byte terminator i.e. empty string");
}

void
kstrchr_test_no_match (void) {
  char* ret = kstrchr("derridadumb", 'x');
  is(ret, NULL, "returns an empty string when no match");
}

void
kstrcmp_test (void) {
  ok(kstrcmp("thomassowell", "thomassowell") == 0, "returns 0 if s1 and s2 are equal");

  ok(
    kstrcmp("thomassowell", "thomasowel") == 4,
    "returns a positive value if s1 is greater than s2"
  );
  ok(kstrcmp("thomasowel", "thomassowell") == -4, "returns a negative value if s1 is less than s2");
}

void
kstrcmp_test_empty (void) {
  ok(kstrcmp("", "") == 0, "returns 0 (eq) when comparing two empty strings");
}

void
kstrcpy_test (void) {
  char  dest[32];
  char* src = "lucrecia_dalt";
  char* ret = kstrcpy(dest, src);

  is(src, dest, "copies the entire string to dest");
  is(ret, dest, "also returns dest");
}

void
kstrcpy_test_empty (void) {
  char  dest[32];
  char* src = "";
  char* ret = kstrcpy(dest, src);

  is(src, dest, "copies the empty string to dest");
  is(ret, dest, "also returns dest");
}

void
kmemcpy_test (void) {
  char  dest[64];
  char* src = "a2FtYWxhX2hhcnJpc19lc3RhYmxpc2htZW50X2ZyYXVkX2xvbAo=";

  char* ret = (char*)kmemcpy(dest, src, 52);
  ret[52]   = '\0';

  is(ret, src, "copies all bytes");

  char dest2[64];

  kmemcpy(dest2, src, 16);
  dest2[16] = '\0';
  is("a2FtYWxhX2hhcnJp", dest2, "copies n bytes");
}

void
kmemset_test (void) {
  char  dest[16];
  char* src = "kocani_orkestar";

  char* ptr = (char*)kmemcpy(dest, src, 16);
  ptr[16]   = '\0';

  char* ret = (char*)kmemset(ptr, 'x', 6);
  is(ret, "xxxxxx_orkestar", "sets n bytes");
}

void
kmemmove_test (void) {
  char src[3]  = {'y', 'y', 'y'};
  char dest[8] = {'p', 'o', 'p', 'x', 'x', 'x'};

  char* ret    = (char*)kmemmove(dest, src, 3);
  is(ret, "yyyxxx", "sets n bytes");
}

int
kmemcmp_test (void) {
  char a1[3] = {'y', 'y', 'y'};
  char b1[3] = {'x', 'x', 'x'};
  int  r1    = kmemcmp(a1, b1, 3);
  ok(r1 == 1, "compares");

  char a2[3] = {'x', 'x', 'x'};
  char b2[3] = {'x', 'x', 'x'};
  int  r2    = kmemcmp(a2, b2, 6);
  ok(r2 == -1, "compares");

  char a3[5] = {'x', 'x', 'x', 'x', 'x'};
  char b3[3] = {'x', 'x', 'x'};
  int  r3    = kmemcmp(a3, b3, 3);
  ok(r3 == 0, "compares");
}

void
kmemchr_test (void) {
  char  p[4] = {'1', '2', '3', '\0'};
  char* r    = kmemchr(p, '2', 4);
  ok(p[0] == '1', "does not modify the original pointer");
  ok(r[0] == '2', "returns a pointer starting at the identified byte");
}

void
kmemchr_test_null_byte (void) {
  char  p[4] = {'1', '2', '3', '\0'};
  char* r    = kmemchr(p, '\0', 4);
  is(r, "", "returns a pointer to the null byte terminator i.e. empty string");
}

void
kmemchr_test_no_match (void) {
  char  p[4] = {'1', '2', '3', '\0'};
  char* r    = kmemchr(p, '4', 4);
  is(r, NULL, "returns empty when no match");
}

void
katoi_test (void) {
  int i = katoi("123567");
  ok(i == 123567, "converts an unsigned integer");

  i = katoi("-5893");
  ok(i == -5893, "converts a signed integer");
}

void
kitoa_test (void) {
  char s[64];

  kitoa(123567, s, 10);
  is(s, "123567", "converts an unsigned integer");

  kitoa(-5893, s, 10);
  is(s, "-5893", "converts a signed integer");

  kitoa(4096, s, 16);
  is(s, "1000", "converts a base 16 integer");
}

void
run_string_tests (void) {
  kstrlen_test();
  kstrlen_test_empty();

  kstrchr_test();
  kstrchr_test_null_byte();
  kstrchr_test_no_match();

  kstrcmp_test();
  kstrcmp_test_empty();

  kstrcpy_test();
  kstrcpy_test_empty();

  kmemcpy_test();

  kmemset_test();

  kmemmove_test();

  kmemcmp_test();

  kmemchr_test();
  kmemchr_test_null_byte();
  kmemchr_test_no_match();

  katoi_test();
  kitoa_test();
}
