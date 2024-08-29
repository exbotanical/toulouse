#ifndef ELF_H
#define ELF_H

#include "common/types.h"

#define ELF_MAGIC 0x464C457FU
#define EI_NIDENT 16

typedef uint32_t elf32_addr;
typedef uint16_t elf32_half;
typedef uint32_t elf32_off;
typedef int32_t  elf32_sword;
typedef uint32_t elf32_word;

// https://docs.oracle.com/cd/E19683-01/816-1386/chapter6-43405/index.html
typedef struct {
  elf32_addr e_magic;
  uint8_t    e_ident[EI_NIDENT];
  elf32_half e_type;
  elf32_half e_machine;
  elf32_word e_version;
  elf32_addr e_entry;
  elf32_off  e_phoff;
  elf32_off  e_shoff;
  elf32_word e_flags;
  elf32_half e_ehsize;
  elf32_half e_phentsize;
  elf32_half e_phnum;
  elf32_half e_shentsize;
  elf32_half e_shnum;
  elf32_half e_shstrndx;
} elf32_header;

#endif /* ELF_H */
