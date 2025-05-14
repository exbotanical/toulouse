#ifndef PROC_LOCK_H
#define PROC_LOCK_H

typedef struct {
  bool locked;
  bool wanted;
} resource_t;

void     lock_resource(resource_t *resource);
void     unlock_resource(resource_t *resource);
retval_t lock_area(unsigned int type);
retval_t unlock_area(unsigned int type);

#endif /* PROC_LOCK_H */
