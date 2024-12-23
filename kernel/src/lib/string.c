#include "lib/string.h"

static char* CHARSET = "0123456789abcdefghijklmnopqrstuvwxyz";

size_t
kstrlen (const char* s) {
  size_t l = 0;
  while (*s++) {
    l++;
  }
  return l;
}

char*
kstrchr (const char* s, char c) {
  do {
    if (*s == c) {
      return (char*)s;
    }
  } while (*s++);

  return (0);
}

int
kstrcmp (const char* s1, const char* s2) {
  while (*s1 && *s1 == *s2) {
    *s1++;
    *s2++;
  }

  return *s1 - *s2;
}

char*
kstrcpy (char* dest, const char* src) {
  char* ret = dest;
  while ((*dest++ = *src++) != '\0');
  return ret;
}

void*
kmemcpy (void* dest, const void* source, size_t bytes) {
  char* ret = dest;
  for (unsigned int i = 0; i < bytes; i++) {
    ((char*)dest)[i] = ((const char*)source)[i];
  }

  return ret;
}

void*
kmemset (void* s, int c, size_t bytes) {
  void* ret = s;
  for (unsigned int i = 0; i < bytes; i++) {
    ((char*)s)[i] = c;
  }

  return ret;
}

void*
kmemmove (void* dest, const void* src, size_t bytes) {
  char tmp[bytes];

  // 2 loops - must be able to overlap
  for (unsigned int i = 0; i < bytes; i++) {
    ((char*)tmp)[i] = ((char*)src)[i];
  }

  for (unsigned int i = 0; i < bytes; i++) {
    ((char*)dest)[i] = ((char*)src)[i];
  }

  return dest;
}

int
kmemcmp (const void* s1, const void* s2, size_t bytes) {
  const char* ca = (const char*)s1;
  const char* cb = (const char*)s2;

  while (bytes-- > 0) {
    if (*ca++ != *cb++) {
      return *ca - *cb;
    }
  }

  return 0;
}

void*
kmemchr (void* s, int value, size_t bytes) {
  unsigned char* p = (unsigned char*)s;
  unsigned char  v = (unsigned char)value;

  for (unsigned int i = 0; i < bytes; i++) {
    if (*p == v) {
      return p;
    }
    *p++;
  }

  return NULL;
}

char*
kitoa (int value, char* buf, int radix) {
  char* ret = buf;
  char  tmp[64];
  int   idx = 0;

  if (value < 0) {
    *buf++ = '-';
    value  = -value;
  }

  if (value == 0) {
    *buf++ = '0';
    *buf   = 0;
    return ret;
  }

  while (value > 0) {
    tmp[idx++]  = CHARSET[value % radix];
    value      /= radix;
  }

  while (idx > 0) {
    *buf++ = tmp[--idx];
  }
  *buf = 0;
  return ret;
}

int
katoi (char* str) {
  int      i      = 0;
  uint32_t factor = 1;

  if (*str == '-') {
    factor = -1;
    str++;
  }

  if (*str == '+') {
    str++;
  }

  while (*str) {
    if (*str < '0' || *str > '9') {
      break;
    }
    i *= 10;
    i += *str - '0';
    str++;
  }

  return i * factor;
}
