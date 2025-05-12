#ifndef KLIB_FWDLIST_H
#define KLIB_FWDLIST_H

#include "lib/compiler.h"
#include "lib/types.h"

/**
 * Implements an append-only singly-linked forward list.
 *
 * For more info, see: https://kernelnewbies.org/FAQ/LinkedLists
 */

/**
 * Initializes a new list_head_t.
 */
#define flist_head(name)                {&(name), &(name)}

/**
 * Defines a new list.
 */
#define define_list(name)               flist_head_t name = flist_head(name)

/**
 * Extrapolates the list node data.
 */
#define flist_entry(ptr, type, member)  containerof(ptr, type, member)

/**
 * Retrieves the data entry (`member`) of the first list node after `ptr`.
 */
#define flist_first(ptr, type, member)  flist_entry((ptr)->next, type, member)

/**
 * Determines whether the given list node is the last in the list.
 */
#define flist_is_last(el, head, member) (&(el)->member == NULL)

/**
 * Retrieves the next node in the list after `el`.
 */
#define flist_next(el, head, member)    flist_entry((el)->member.next, typeof(*el), member)

/**
 * Generates a loop beginning with list node `head`. Each element is placed in `el`.
 */
#define flist_foreach(el, head)         for (el = (head)->next; el; el = el->next)

#define flist_foreach_entry(el, head, member)                                         \
  for (el = flist_first(head, typeof(*el), member); !flist_is_last(el, head, member); \
       el = flist_next(el, head, member))

typedef struct flist_head flist_head_t;

struct flist_head {
  flist_head_t *next;
};

/**
 * Initializes a new list given an entry.
 *
 * @param entry
 */
static inline void
flist_init (flist_head_t *entry) {
  entry->next = (flist_head_t *)NULL;
}

/**
 * Inserts a list node `entry` between nodes `prev` and `next`.
 *
 * @param entry
 * @param prev
 * @param next
 */
static inline void
flist_insert (flist_head_t *entry, flist_head_t *prev, flist_head_t *next) {
  // TODO: Implement assert and cmp prev != next (for singly fwd, this is an invariant - cannot
  // insert into self)
  prev->next  = entry;
  entry->next = next;
}

/**
 * Appends a node `entry` at the end of list `head`.
 *
 * @param entry
 * @param head
 */
static inline void
flist_append (flist_head_t *entry, flist_head_t *head) {
  while (head->next) {
    head = head->next;
  }

  head->next = entry;
}

#endif /* KLIB_FWDLIST_H */
