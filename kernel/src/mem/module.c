#include "mem/module.h"

#include "lib/k_math.h"
#include "mem/base.h"

static inline uint32_t
get_module_page (uint32_t phys) {
  return phys / PAGE_SZ;
}

bool
is_module_page (uint32_t idx) {
  for (uint32_t i = 0; i < num_modules; i++) {
    if (idx >= get_module_page(modules[i].start) && idx <= get_module_page(modules[i].end)) {
      return true;
    }
  }

  return false;
}

void
module_init (multiboot_info_t* mbi) {
  num_modules = min(MAX_MODULES, mbi->mods_count);

  for (uint32_t i = 0; i < num_modules; i++) {
    modules[i].start   = mbi->mods[i].start;
    modules[i].end     = mbi->mods[i].end;
    modules[i].cmdline = mbi->mods[i].cmdline;
  }
}
