#include "tests.h"

u_int32_t image_start = 0;
u_int32_t image_end   = 0;
u_int32_t text_end    = 0;
u_int32_t data_end    = 0;

void*
kmalloc (size_t s) {
  return (void*)NULL;
}

int
main () {
  plan(9);

  // run_spinlock_tests();
  run_buddy_tests();

  done_testing();
}
