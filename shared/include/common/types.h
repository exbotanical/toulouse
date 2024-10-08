#ifndef TYPES_H
#define TYPES_H

typedef _Bool bool;
#define true  1
#define false 0

#define NULL  ((void *)0)

typedef signed char      int8_t;
typedef signed short     int16_t;
typedef signed int       int32_t;
typedef signed long long int64_t;

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;
// TODO: fix?
typedef unsigned int       uintptr_t;

typedef uint32_t size_t;

#endif /* TYPES_H */
