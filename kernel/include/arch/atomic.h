#ifndef ATOMIC_H
#define ATOMIC_H

#define __X86_CASE_B 1
#define __X86_CASE_W 2
#define __X86_CASE_L 4

#define atomic_xchg(op, ptr, arg)                       \
  __extension__({                                       \
    __typeof__(*(ptr)) __xchg_ret__ = (arg);            \
    switch (sizeof(*(ptr))) {                           \
      case __X86_CASE_B:                                \
        asm volatile("lock; x" #op "b %b0, %1\n"        \
                     : "+q"(__xchg_ret__), "+m"(*(ptr)) \
                     :                                  \
                     : "memory", "cc");                 \
        break;                                          \
      case __X86_CASE_W:                                \
        asm volatile("lock; x" #op "w %w0, %1\n"        \
                     : "+r"(__xchg_ret__), "+m"(*(ptr)) \
                     :                                  \
                     : "memory", "cc");                 \
        break;                                          \
      case __X86_CASE_L:                                \
        asm volatile("lock; x" #op "l %0, %1\n"         \
                     : "+r"(__xchg_ret__), "+m"(*(ptr)) \
                     :                                  \
                     : "memory", "cc");                 \
        break;                                          \
    }                                                   \
    __xchg_ret__;                                       \
  })

#define atomic_add(ptr, inc)                                                              \
  __extension__({                                                                         \
    __typeof__(*(ptr)) __add_ret__ = (inc);                                               \
    switch (sizeof(*(ptr))) {                                                             \
      case __X86_CASE_B:                                                                  \
        asm volatile("lock; addb %b1, %0\n" : "+m"(*(ptr)) : "qi"(inc) : "memory", "cc"); \
        break;                                                                            \
      case __X86_CASE_W:                                                                  \
        asm volatile("lock; addw %w1, %0\n" : "+m"(*(ptr)) : "ri"(inc) : "memory", "cc"); \
        break;                                                                            \
      case __X86_CASE_L:                                                                  \
        asm volatile("lock; addl %1, %0\n" : "+m"(*(ptr)) : "ri"(inc) : "memory", "cc");  \
        break;                                                                            \
    }                                                                                     \
    __add_ret__;                                                                          \
  })

#endif /* ATOMIC_H */
