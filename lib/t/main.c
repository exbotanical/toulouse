#include "tests.h"

int
main () {
  plan(148);

  run_string_tests();
  run_flist_tests();
  run_list_tests();
  run_ctype_tests();

  done_testing();
}
