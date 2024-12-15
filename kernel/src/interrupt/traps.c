#include "interrupt/traps.h"

#include "drivers/console/vga.h"
#include "fs/elf.h"
#include "mem/base.h"
#include "mem/paging.h"
#include "mem/segments.h"

trap_t traps_table[NUM_EXCEPTIONS] = {
  {"Division By Zero",                      trap_divide_error,                0},
  {"Debug",                                 trap_debug,                       0},
  {"Non Maskable Interrupt",                trap_nmi_interrupt,               0},
  {"Breakpoint",                            trap_breakpoint,                  0},
  {"Into Detected Overflow",                trap_overflow,                    0},
  {"Out of Bounds",                         trap_bound,                       0},
  {"Invalid Opcode",                        trap_invalid_opcode,              0},
  {"Device Not Available (No Coprocessor)", trap_no_math_coprocessor,         0},
  {"Double Fault",                          trap_double_fault,                1},
  {"Coprocessor Segment Overrun",           trap_coprocessor_segment_overrun, 0},
  {"Invalid TSS",                           trap_invalid_tss,                 1},
  {"Segment Not Present",                   trap_segment_not_present,         1},
  {"Stack-Segment Fault",                   trap_stack_segment_fault,         1},
  {"General Protection Fault",              trap_general_protection,          1},
  {"Page Fault",                            trap_page_fault,                  1},
  {"Intel reserved",                        trap_reserved,                    0},
  {"x87 FPU Floating-Point Error",          trap_floating_point_error,        0},
  {"Alignment Check",                       trap_alignment_check,             1},
  {"Machine Check",                         trap_machine_check,               0},
  {"SIMD Floating-Point Exception",         trap_simd_fault,                  0},
  {"Intel reserved",                        trap_reserved,                    0},
  {"Intel reserved",                        trap_reserved,                    0},
  {"Intel reserved",                        trap_reserved,                    0},
  {"Intel reserved",                        trap_reserved,                    0},
  {"Intel reserved",                        trap_reserved,                    0},
  {"Intel reserved",                        trap_reserved,                    0},
  {"Intel reserved",                        trap_reserved,                    0},
  {"Intel reserved",                        trap_reserved,                    0},
  {"Intel reserved",                        trap_reserved,                    0},
  {"Intel reserved",                        trap_reserved,                    0},
  {"Intel reserved",                        trap_reserved,                    0},
  {"Intel reserved",                        trap_reserved,                    0}
};

static const char*
elf_lookup_symbol (unsigned int addr) {
  // elf32_shdr * vsymtab, *vstrtab;
  // elf32_sym*   sym;
  // unsigned int n;

  // vsymtab = (elf32_shdr*)P2V((unsigned int)symtab);
  // vstrtab = (elf32_shdr*)P2V((unsigned int)strtab);
  // sym     = (elf32_sym*)P2V(vsymtab->sh_addr);
  // for (n = 0; n < vsymtab->sh_size / sizeof(elf32_sym); n++, sym++) {
  //   if (ELF32_ST_TYPE(sym->st_info) != STT_FUNC) {
  //     continue;
  //   }
  //   if (addr >= sym->st_value && addr < (sym->st_value + sym->st_size)) {
  //     return (const char*)P2V(vstrtab->sh_addr) + sym->st_name;
  //   }
  // }
  return NULL;
}

static void
print_stacktrace (void) {
  vgaprintf("%s\n", "stacktrace:");
  int           n;
  unsigned int* esp;
  unsigned int  addr;

  asm volatile("movl %%esp, %0" : "=r"(esp));

  esp += (sizeof(sigcontext_t) / sizeof(unsigned int)) - 5;
  esp  = (unsigned int*)P2V((unsigned int)esp);
  for (n = 1; n <= 32; n++) {
    vgaprintf(" %08x", *esp);
    esp++;
    if (!(n % 8)) {
      vgaprintf("%s", "\n");
    }
  }
  vgaprintf("%s", "Kernel backtrace:\n");
  asm volatile("movl %%esp, %0" : "=r"(esp));

  esp += (sizeof(sigcontext_t) / sizeof(unsigned int)) - 5;
  esp  = (unsigned int*)P2V((unsigned int)esp);

  const char* str;
  for (n = 0; n < 256; n++) {
    addr = *esp;
    str  = elf_lookup_symbol(addr);
    if (str) {
      vgaprintf("<0x%08x> %s()\n", addr, str);
    }
    esp++;
  }
}

static bool
dump_registers (unsigned int trap, sigcontext_t* sc) {
  bool is_page_fault = trap == 14;

  if (is_page_fault) {
    unsigned int cr2;
    asm volatile("movl %%cr2, %0" : "=r"(cr2));
    vgaprintf(
      "%s at 0x%08x (%s) with errcode 0x%02x%s",
      traps_table[trap].name,
      cr2,
      sc->err & PAGE_FAULT_WRIT ? "writing" : "reading",
      sc->err,
      sc->err & PAGE_FAULT_USRMOD ? "\n" : " in kernel mode.\n"
    );
  } else {
    vgaprintf("[ERROR]: %s", traps_table[trap].name);
    if (traps_table[trap].errcode) {
      vgaprintf(": error code 0x%08x (0b%b)", sc->err, sc->err);
    }
    vgaprintf("%s", "\n");
  }

  vgaprintf(
    " cs: 0x%04x\teip: 0x%08x\tefl: 0x%08x\t ss: 0x%08x\tesp: 0x%08x\n",
    sc->cs,
    sc->eip,
    sc->eflags,
    sc->og_ss,
    sc->og_esp
  );
  vgaprintf(
    "eax: 0x%08x\tebx: 0x%08x\tecx: 0x%08x\tedx: 0x%08x\n",
    sc->eax,
    sc->ebx,
    sc->ecx,
    sc->edx
  );
  vgaprintf(
    "esi: 0x%08x\tedi: 0x%08x\tesp: 0x%08x\tebp: 0x%08x\n",
    sc->esi,
    sc->edi,
    sc->esp,
    sc->ebp
  );
  vgaprintf(" ds: 0x%04x\t es: 0x%04x\t fs: 0x%04x\t gs: 0x%04x\n", sc->ds, sc->es, sc->fs, sc->gs);

  if (sc->cs == KERNEL_CS) {
    print_stacktrace();
  }

  bool was_in_kernel_mode = sc->cs == KERNEL_CS;
  if (was_in_kernel_mode) {
    print_stacktrace();
    return false;
  }

  return true;
}

void
trap_handle (unsigned int trap, sigcontext_t sc) {
  traps_table[trap].handler(trap, &sc);
  sc.err = -sc.err;
}

void
trap_divide_error (unsigned int trap, sigcontext_t* sc) {
  if (!dump_registers(trap, sc)) {
    // k_panic("Failed to dump registers in trap_divide_error");
  }
}

void
trap_debug (unsigned int trap, sigcontext_t* sc) {}

void
trap_nmi_interrupt (unsigned int trap, sigcontext_t* sc) {}

void
trap_breakpoint (unsigned int trap, sigcontext_t* sc) {}

void
trap_overflow (unsigned int trap, sigcontext_t* sc) {}

void
trap_bound (unsigned int trap, sigcontext_t* sc) {}

void
trap_invalid_opcode (unsigned int trap, sigcontext_t* sc) {
  vgaprintf("ERR: %s %d \n", "FUCK!", trap);
}

void
trap_no_math_coprocessor (unsigned int trap, sigcontext_t* sc) {}

void
trap_double_fault (unsigned int trap, sigcontext_t* sc) {}

void
trap_coprocessor_segment_overrun (unsigned int trap, sigcontext_t* sc) {}

void
trap_invalid_tss (unsigned int trap, sigcontext_t* sc) {}

void
trap_segment_not_present (unsigned int trap, sigcontext_t* sc) {}

void
trap_stack_segment_fault (unsigned int trap, sigcontext_t* sc) {}

void
trap_general_protection (unsigned int trap, sigcontext_t* sc) {}

void
trap_page_fault (unsigned int trap, sigcontext_t* sc) {}

void
trap_reserved (unsigned int trap, sigcontext_t* sc) {}

void
trap_floating_point_error (unsigned int trap, sigcontext_t* sc) {}

void
trap_alignment_check (unsigned int trap, sigcontext_t* sc) {}

void
trap_machine_check (unsigned int trap, sigcontext_t* sc) {}

void
trap_simd_fault (unsigned int trap, sigcontext_t* sc) {}
