#ifndef ELF_H
#define ELF_H
// See: https://refspecs.linuxfoundation.org/elf/elf.pdf

#include "common/compiler.h"
#include "common/types.h"

// #define ELF_MAGIC    0x464C457FU
#define ELFMAG0      0x7f
#define ELFMAG1      'E'
#define ELFMAG2      'L'
#define ELFMAG3      'F'
#define ELFMAG       "\177ELF"

#define EI_MAG0      0
#define EI_MAG1      1
#define EI_MAG2      2
#define EI_MAG3      3
// File class
#define EI_CLASS     4
// Data encoding
#define EI_DATA      5
// File version
#define EI_VERSION   6
// Start of padding bytes
#define EI_PAD       7
// Size of e_ident[]
#define EI_NIDENT    16

// Invalid class
#define ELFCLASSNONE 0
// 32-bit objects
#define ELFCLASS32   1
// 64-bit objects
#define ELFCLASS64   2

// Invalid data encoding
#define ELFDATANONE  0
// Little endian
#define ELFDATA2LSB  1
// Big endian
#define ELFDATA2MSB  2

// No file type
#define ET_NONE      0
// Relocatable file
#define ET_REL       1
// Executable file
#define ET_EXEC      2
// Shared object file
#define ET_DYN       3
// Core file
#define ET_CORE      4

// Invalid version
#define EV_NONE      0
// Current version
#define EV_CURRENT   1

// Machine types
#define EM_NONE      0
#define EM_M32       1
#define EM_SPARC     2
#define EM_386       3
#define EM_68K       4
#define EM_88K       5
#define EM_860       7
#define EM_MIPS      8

// Program header types
#define PT_NULL      0
#define PT_LOAD      1
#define PT_DYNAMIC   2
#define PT_INTERP    3
#define PT_NOTE      4
#define PT_SHLIB     5
#define PT_PHDR      6
#define PT_TLS       7
#define PT_LOOS      0x60000000
#define PT_HIOS      0x6fffffff
#define PT_LOPROC    0x70000000
#define PT_HIPROC    0x7fffffff

#define STB_LOCAL    0
#define STB_GLOBAL   1
#define STB_WEAK     2
#define STB_NUM      3

#define STT_NOTYPE   0
#define STT_OBJECT   1
#define STT_FUNC     2
#define STT_SECTION  3
#define STT_FILE     4
#define STT_NUM      5

#define SHT_NULL     0
#define SHT_PROGBITS 1
#define SHT_SYMTAB   2
#define SHT_STRTAB   3
#define SHT_RELA     4
#define SHT_HASH     5
#define SHT_DYNAMIC  6
#define SHT_NOTE     7
#define SHT_NOBITS   8
#define SHT_REL      9
#define SHT_SHLIB    10
#define SHT_DYNSYM   11
#define SHT_NUM      12

typedef uint32_t elf32_addr;
typedef uint16_t elf32_half;
typedef uint32_t elf32_off;
typedef int32_t  elf32_sword;
typedef uint32_t elf32_word;

// https://docs.oracle.com/cd/E19683-01/816-1386/chapter6-43405/index.html
typedef struct {
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
} packed elf32_hdr;

typedef struct {
  elf32_word p_type;
  elf32_off  p_offset;
  elf32_addr p_vaddr;
  elf32_addr p_paddr;
  elf32_word p_filesz;
  elf32_word p_memsz;
  elf32_word p_flags;
  elf32_word p_align;
} packed elf32_phdr;

typedef struct {
  elf32_word sh_name;
  elf32_word sh_type;
  elf32_word sh_flags;
  elf32_addr sh_addr;
  elf32_off  sh_offset;
  elf32_word sh_size;
  elf32_word sh_link;
  elf32_word sh_info;
  elf32_word sh_addralign;
  elf32_word sh_entsize;
} elf32_shdr;

typedef struct {
  elf32_word    st_name;
  elf32_addr    st_value;
  elf32_word    st_size;
  unsigned char st_info;
  unsigned char st_other;
  elf32_half    st_shndx;
} elf32_sym;

static bool
elf_is_valid_sig (elf32_hdr* elf) {
  return elf->e_ident[EI_MAG0] == ELFMAG0 && elf->e_ident[EI_MAG1] == ELFMAG1
         && elf->e_ident[EI_MAG2] == ELFMAG2 && elf->e_ident[EI_MAG3] == ELFMAG3;
}

static bool
elf_is_valid_class (elf32_hdr* elf) {
  return elf->e_ident[EI_CLASS] == ELFCLASS32;
}

static bool
elf_is_valid_encoding (elf32_hdr* elf) {
  return elf->e_ident[EI_DATA] == ELFDATA2LSB;
}

static bool
elf_is_valid_version (elf32_hdr* elf) {
  return elf->e_ident[EI_VERSION] == EV_CURRENT && elf->e_version == EV_CURRENT;
}

static bool
elf_is_valid_machine (elf32_hdr* elf) {
  return elf->e_machine == EM_386;
}

static bool
elf_is_executable (elf32_hdr* elf) {
  // TODO: check e_entry <= max mem addr
  return elf->e_type == ET_EXEC;
}

static bool
elf_has_phdr (elf32_hdr* elf) {
  return elf->e_phoff != 0;
}

static bool
elf_is_valid_executable (elf32_hdr* elf) {
  return elf_is_valid_sig(elf) && elf_is_valid_class(elf) && elf_is_valid_encoding(elf)
         && elf_is_valid_version(elf) && elf_is_valid_machine(elf) && elf_is_executable(elf)
         && elf_has_phdr(elf);
}

#endif /* ELF_H */
