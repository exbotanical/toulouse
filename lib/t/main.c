#include "tests.h"

int
main () {
  plan(92);

  run_string_tests();
  run_flist_tests();
  run_list_tests();

  done_testing();
}
