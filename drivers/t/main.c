#include "mocks.h"
#include "stubs.h"
#include "tests.h"

int
main () {
  plan(269 + 267);
  run_device_tests();
  run_charq_tests();

  done_testing();
}
