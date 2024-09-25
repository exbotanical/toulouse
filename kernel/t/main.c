#include "arch/idt.h"
#include "tests.h"

u_int32_t image_start = 0;
u_int32_t image_end   = 0;

void
load_idt (idtr_desc_t* idtr) {}

int
main () {
  plan(78);

  run_string_tests();
  run_list_tests();
  run_spinlock_tests();

  done_testing();
}
