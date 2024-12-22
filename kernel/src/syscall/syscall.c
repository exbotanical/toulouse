#include "syscall/syscall.h"

int
syscall_exec (
  unsigned int   num,
  int            arg_1,
  int            arg_2,
  int            arg_3,
  int            arg_4,
  int            arg_5,
  sig_context_t* sc
) {
  return 1;
}
