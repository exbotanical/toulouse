#include "mem/bss.h"

#include "lib/string.h"
#include "mem/base.h"

void
bss_init (void) {
  kmemset((void*)(&data_end), 0, KERNEL_BSS_SIZE);
}
