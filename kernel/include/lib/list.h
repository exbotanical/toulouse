#ifndef LIST_H
#define LIST_H

#include "common/compiler.h"

/**
 * Implements a circular doubly-linked list. Based on the list implementation used throughout the
 * Linux kernel.
 *
 * For more info, see: https://kernelnewbies.org/FAQ/LinkedLists
 */

/**
 * Extrapolates the list node data.
 */
#define list_entry(ptr, type, member) containerof(ptr, type, member)

/**
 * Initializes a new list_head_t.
 */
#define list_head(name)               {&(name), &(name)}

/**
 * Defines a new list.
 */
#define define_list(name)             list_head_t name = list_head(name)

/**
 * Generates a loop beginning with list node `head`. Each element is placed in `pos`.
 */
#define list_foreach(pos, head)       for (pos = (head)->next; pos != (head); pos = pos->next)

typedef struct list_head list_head_t;

struct list_head {
  list_head_t *prev;
  list_head_t *next;
};

/**
 * Initializes a new list given an entry.
 *
 * @param entry
 */
static inline void
list_init (list_head_t *entry) {
  entry->next = entry;
  entry->prev = entry;
}

/**
 * Joins two list nodes together.
 *
 * @param first
 * @param second
 */
static inline void
list_join (list_head_t *first, list_head_t *second) {
  first->next  = second;
  second->prev = first;
}

/**
 * Inserts a list node `entry` between nodes `prev` and `next`.
 *
 * @param entry
 * @param prev
 * @param next
 */
static inline void
list_insert (list_head_t *entry, list_head_t *prev, list_head_t *next) {
  prev->next  = entry;
  entry->prev = prev;
  entry->next = next;
  next->prev  = entry;
}

/**
 * Appends a node `entry` at the end of node `prev`.
 *
 * @param entry
 * @param prev
 */
static inline void
list_append (list_head_t *entry, list_head_t *prev) {
  list_insert(entry, prev, prev->next);
}

/**
 * Removes a list node `entry`.
 *
 * @param entry
 */
static inline void
list_remove (list_head_t *entry) {
  list_join(entry->prev, entry->next);
  // Make sure the removed node isn't pointing back at the original list
  list_init(entry);
}

#endif /* LIST_H */
