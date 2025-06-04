#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#define bool  _Bool
#define true  1
#define false 0

typedef enum { RET_OK = true, RET_FAIL = false } retval_t;

#define NULL ((void *)0)

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

typedef uint16_t deviceno_t;

typedef uint32_t size_t;

/**
 * Process id type
 */
typedef uint32_t pid_t;
#define __pid_t_defined 1

#endif /* COMMON_TYPES_H */
