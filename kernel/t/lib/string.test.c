#include <stdio.h>

#include "../tests.h"
#include "lib/k_string.h"

void
k_strlen_test (void) {
  char* s = "katenv binasu\0";
  ok(k_strlen(s) == 13, "returns the number of bytes in the string pointed to by s");
}

void
k_strlen_test_empty (void) {
  char* s = "\0";
  ok(k_strlen(s) == 0, "returns zero for empty string (and ignores terminating null byte)");
}

void
k_strchr_test (void) {
  char* og  = "helelo";
  char* ret = k_strchr(og, 'e');

  is(ret, "elelo", "returns a char pointer starting at the first instance of the specified char");
  is(og, "helelo", "does not modify the original char pointer");
  *ret++;
  is(ret, "lelo", "returns a new char pointer");
}

void
k_strchr_test_null_byte (void) {
  char* ret = k_strchr("postmodernism sux", '\0');
  is(ret, "", "returns a pointer to the null byte terminator i.e. empty string");
}

void
k_strchr_test_no_match (void) {
  char* ret = k_strchr("derridadumb", 'x');
  is(ret, NULL, "returns an empty string when no match");
}

void
k_strcmp_test (void) {
  ok(k_strcmp("thomassowell", "thomassowell") == 0, "returns 0 if s1 and s2 are equal");

  ok(
    k_strcmp("thomassowell", "thomasowel") == 4,
    "returns a positive value if s1 is greater than s2"
  );
  ok(
    k_strcmp("thomasowel", "thomassowell") == -4,
    "returns a negative value if s1 is less than s2"
  );
}

void
k_strcmp_test_empty (void) {
  ok(k_strcmp("", "") == 0, "returns 0 (eq) when comparing two empty strings");
}

void
k_strcpy_test (void) {
  char  dest[32];
  char* src = "lucrecia_dalt";
  char* ret = k_strcpy(dest, src);

  is(src, dest, "copies the entire string to dest");
  is(ret, dest, "also returns dest");
}

void
k_strcpy_test_empty (void) {
  char  dest[32];
  char* src = "";
  char* ret = k_strcpy(dest, src);

  is(src, dest, "copies the empty string to dest");
  is(ret, dest, "also returns dest");
}

void
k_memcpy_test (void) {
  char  dest[64];
  char* src = "a2FtYWxhX2hhcnJpc19lc3RhYmxpc2htZW50X2ZyYXVkX2xvbAo=";

  char* ret = (char*)k_memcpy(dest, src, 52);
  ret[52]   = '\0';

  is(ret, src, "copies all bytes");

  char dest2[64];

  k_memcpy(dest2, src, 16);
  dest2[16] = '\0';
  is("a2FtYWxhX2hhcnJp", dest2, "copies n bytes");
}

void
k_memset_test (void) {
  char  dest[16];
  char* src = "kocani_orkestar";

  char* ptr = (char*)k_memcpy(dest, src, 16);
  ptr[16]   = '\0';

  char* ret = (char*)k_memset(ptr, 'x', 6);
  is(ret, "xxxxxx_orkestar", "sets n bytes");
}

void
k_memmove_test (void) {
  char src[3]  = {'y', 'y', 'y'};
  char dest[8] = {'p', 'o', 'p', 'x', 'x', 'x'};

  char* ret    = (char*)k_memmove(dest, src, 3);
  is(ret, "yyyxxx", "sets n bytes");
}

int
k_memcmp_test (void) {
  char a1[3] = {'y', 'y', 'y'};
  char b1[3] = {'x', 'x', 'x'};
  int  r1    = k_memcmp(a1, b1, 3);
  ok(r1 == 1, "compares");

  char a2[3] = {'x', 'x', 'x'};
  char b2[3] = {'x', 'x', 'x'};
  int  r2    = k_memcmp(a2, b2, 6);
  ok(r2 == -1, "compares");

  char a3[5] = {'x', 'x', 'x', 'x', 'x'};
  char b3[3] = {'x', 'x', 'x'};
  int  r3    = k_memcmp(a3, b3, 3);
  ok(r3 == 0, "compares");
}

void
k_memchr_test (void) {
  char  p[4] = {'1', '2', '3', '\0'};
  char* r    = k_memchr(p, '2', 4);
  ok(p[0] == '1', "does not modify the original pointer");
  ok(r[0] == '2', "returns a pointer starting at the identified byte");
}

void
k_memchr_test_null_byte (void) {
  char  p[4] = {'1', '2', '3', '\0'};
  char* r    = k_memchr(p, '\0', 4);
  is(r, "", "returns a pointer to the null byte terminator i.e. empty string");
}

void
k_memchr_test_no_match (void) {
  char  p[4] = {'1', '2', '3', '\0'};
  char* r    = k_memchr(p, '4', 4);
  is(r, NULL, "returns empty when no match");
}

void
run_string_tests (void) {
  k_strlen_test();
  k_strlen_test_empty();

  k_strchr_test();
  k_strchr_test_null_byte();
  k_strchr_test_no_match();

  k_strcmp_test();
  k_strcmp_test_empty();

  k_strcpy_test();
  k_strcpy_test_empty();

  k_memcpy_test();

  k_memset_test();

  k_memmove_test();

  k_memcmp_test();

  k_memchr_test();
  k_memchr_test_null_byte();
  k_memchr_test_no_match();
}
