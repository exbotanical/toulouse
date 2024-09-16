#include "tests.h"

int
main () {
  plan(64);

  run_string_tests();
  run_list_tests();

  done_testing();
}
