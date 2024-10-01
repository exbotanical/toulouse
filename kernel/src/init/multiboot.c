#include "init/multiboot.h"

#include "lib/string.h"

void
multiboot_init (unsigned int magic, unsigned int mbi_ptr) {
  multiboot_info_t* mbi = (multiboot_info_t*)mbi_ptr;
}
