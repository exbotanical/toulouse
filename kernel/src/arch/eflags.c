#include "arch/eflags.h"

#include "lib/compiler.h"

overridable uint32_t
eflags_get (void) {
  unsigned int eflags;
  asm volatile("pushfl\n\t"
               "popl %0"
               : "=r"(eflags)
               :
               : "memory");
  return eflags;
}

overridable void
eflags_set (uint32_t eflags) {
  asm volatile("pushl %0; popfl\n\t" : : "r"(eflags) : "memory");
}
