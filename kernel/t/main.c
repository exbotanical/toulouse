#include "tests.h"

u_int32_t image_start = 0;
u_int32_t image_end   = 0;
u_int32_t text_end    = 0;
u_int32_t data_end    = 0;

int
main () {
  plan(369);

  run_string_tests();
  run_flist_tests();
  run_list_tests();
  run_spinlock_tests();
  run_device_tests();
  run_paging_tests();

  done_testing();
}
