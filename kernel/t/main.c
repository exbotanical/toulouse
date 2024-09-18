#include "tests.h"

int
main () {
  plan(72);

  run_string_tests();
  run_list_tests();
  run_spinlock_tests();

  done_testing();
}
