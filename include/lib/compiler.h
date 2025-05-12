#ifndef COMMON_COMPILER_H
#define COMMON_COMPILER_H

/**
 * Optimizations which tell the compiler whether a branch is likely, effectively arranging the code
 * such that the likeliest branch is executed without performing a jmp instruction (and
 * thereby flushing the processor pipeline).
 *
 */
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#ifndef offsetof
/**
 * Expands the address of the `member` field qua the `type` data structure. To obtain the offset, we
 * essentially take the address of `member` from a NULL pointer cast to `type`.
 */
#  define offsetof(type, member) ((unsigned int)&((type *)0)->member)
#endif

/**
 * Casts a member of a structure out to the containing structure.
 * For example, given `ptr` (a pointer to the member), we can extrapolate the address of `member` in
 * the containing struct `type`.
 *
 * Note, see gcc expr: https://gcc.gnu.org/onlinedocs/gcc/Statement-Exprs.html
 */
#define containerof(ptr, type, member)                 \
  ({                                                   \
    const typeof(((type *)0)->member) *__mptr = (ptr); \
    (type *)((char *)__mptr - offsetof(type, member)); \
  })

#define packed         __attribute__((__packed__))

#define aligned(a)     __attribute__((aligned((a))))

/**
 * Prevents compiler optimizations and ensures the value passed as a parameter is accessed exactly
 * once by the generated code. It works by applying `volatile` to the storage location.
 *
 * See: https://lwn.net/Articles/508991
 */
#define access_once(x) (*((volatile typeof(x) *)&(x)))

#define noreturn       __attribute__((noreturn))

#endif /* COMMON_COMPILER_H */
