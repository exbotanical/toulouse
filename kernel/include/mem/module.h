#ifndef MEM_MODULE_H
#define MEM_MODULE_H

#include "common/types.h"
#include "init/multiboot.h"
#include "mem/module.h"

#define MAX_MODULES 256

typedef struct {
  uint32_t start;
  uint32_t end;
  uint32_t cmdline;
} module_t;

void module_init(multiboot_info_t* mbi);

/**
 * Returns a boolean indicating whether the given index is a module page index i.e. a page index
 * that would be resolved by a module's physical address.
 *
 * @param idx
 * @return true is a module page index
 * @return false is not a module page index
 */
bool is_module_page(uint32_t idx);

#endif /* MEM_MODULE_H */
