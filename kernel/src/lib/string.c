#include "lib/string.h"

static char* CHARSET = "0123456789abcdefghijklmnopqrstuvwxyz";

size_t
k_strlen (const char* s) {
  size_t l = 0;
  while (*s++) {
    l++;
  }
  return l;
}

char*
k_strchr (const char* s, char c) {
  do {
    if (*s == c) {
      return (char*)s;
    }
  } while (*s++);

  return (0);
}

int
k_strcmp (const char* s1, const char* s2) {
  while (*s1 && *s1 == *s2) {
    *s1++;
    *s2++;
  }

  return *s1 - *s2;
}

char*
k_strcpy (char* dest, const char* src) {
  char* ret = dest;
  while ((*dest++ = *src++) != '\0');
  return ret;
}

void*
k_memcpy (void* dest, const void* source, size_t bytes) {
  char* ret = dest;
  for (unsigned int i = 0; i < bytes; i++) {
    ((char*)dest)[i] = ((const char*)source)[i];
  }

  return ret;
}

void*
k_memset (void* s, int c, size_t bytes) {
  void* ret = s;
  for (unsigned int i = 0; i < bytes; i++) {
    ((char*)s)[i] = c;
  }

  return ret;
}

void*
k_memmove (void* dest, const void* src, size_t bytes) {
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
k_memcmp (const void* s1, const void* s2, size_t bytes) {
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
k_memchr (void* s, int value, size_t bytes) {
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
k_itoa (int value, char* buff, int base) {
  char* ret = buff;
  char  tmp[64];
  int   idx = 0;

  if (value < 0) {
    *buff++ = '-';
    value   = -value;
  }

  if (value == 0) {
    *buff++ = '0';
    *buff   = 0;
    return ret;
  }

  while (value > 0) {
    tmp[idx++]  = CHARSET[value % base];
    value      /= base;
  }

  while (idx > 0) {
    *buff++ = tmp[--idx];
  }
  *buff = 0;
  return ret;
}

int
k_atoi (char* str) {
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
