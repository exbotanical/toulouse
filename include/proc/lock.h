#ifndef PROC_LOCK_H
#define PROC_LOCK_H

#include "lib/types.h"

#define AREA_BH          0x00000001
#define AREA_CALLOUT     0x00000002
#define AREA_TTY_READ    0x00000004
#define AREA_SERIAL_READ 0x00000008

typedef struct {
  bool locked;
  bool wanted;
} resource_t;

void     lock_resource(resource_t *resource);
void     unlock_resource(resource_t *resource);
retval_t lock_area(unsigned int type);
retval_t unlock_area(unsigned int type);

#endif /* PROC_LOCK_H */
