#include <printf.h>
#include <stdio.h>
#include <string.h>

#include "tests.h"

void*
kmalloc (size_t size) {
  return malloc(size);
}

void
kfree (void* ptr) {
  free(ptr);
}

void*
kmemset (void* ptr, int val, size_t len) {
  return memset(ptr, val, len);
}

size_t
kstrlen (const char* s) {
  return strlen(s);
}

char*
kstrcpy (char* dest, const char* src) {
  return strcpy(dest, src);
}

void*
kmemcpy (void* dest, const void* src, size_t bytes) {
  return memcpy(dest, src, bytes);
}

int
sprintf_ (char* buffer, const char* format, ...) {
  return sprintf(buffer, format);
}

int
main () {
  plan(269 + 267);
  run_device_tests();
  run_charq_tests();

  done_testing();
}
