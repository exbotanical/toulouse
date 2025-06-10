#ifndef MEM_MMAN_H
#define MEM_MMAN_H

/*
Protections are chosen from these bits, OR'd together. The
implementation does not necessarily support PROT_EXEC or PROT_WRITE
without PROT_READ. The only guarantees are that no writing will be
allowed without PROT_WRITE and no access will be allowed for PROT_NONE.
*/
typedef enum {
  PROT_NONE  = 0x0, /* Page cannot be accessed */
  PROT_READ  = 0x1, /* Page can be read */
  PROT_WRITE = 0x2, /* Page can be written */
  PROT_EXEC  = 0x4, /* Page can be executed */
} mem_prot;

#endif              /* MEM_MMAN_H */
