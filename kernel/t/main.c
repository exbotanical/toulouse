#include "tests.h"

u_int32_t image_start = 0;
u_int32_t image_end   = 0;
u_int32_t bss_end     = 0;
u_int32_t data_end    = 0;

int
main () {
  plan(77);

  run_string_tests();
  run_list_tests();
  run_spinlock_tests();

  done_testing();
}
