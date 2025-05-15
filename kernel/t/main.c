#include "tests.h"

u_int32_t image_start = 0;
u_int32_t image_end   = 0;
u_int32_t text_end    = 0;
u_int32_t data_end    = 0;

unsigned int
kmalloc (size_t size) {
  return (unsigned int)malloc(size);
}

void
kfree (unsigned int ptr) {
  free((void*)ptr);
}

int
main () {
  plan(37);

  run_spinlock_tests();
  run_proc_lock_tests();

  done_testing();
}
