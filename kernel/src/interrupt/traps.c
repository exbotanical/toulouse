#include "interrupt/traps.h"

#include "debug/panic.h"
#include "drivers/dev/char/tmpcon.h"
#include "fs/elf.h"
#include "interrupt/pit.h"
#include "kernel.h"
#include "mem/base.h"
#include "mem/page.h"
#include "mem/segments.h"

#define DUMP_REG_OR_FAIL(trap_num, sc)                 \
  if (dump_trap_registers(trap_num, sc) == RET_FAIL) { \
    kpanic("%s", "Failed to dump registers");          \
  }

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
  elf32_shdr* vsymtab = (elf32_shdr*)P2V((unsigned int)symtab);
  elf32_shdr* vstrtab = (elf32_shdr*)P2V((unsigned int)strtab);
  elf32_sym*  sym     = (elf32_sym*)P2V(vsymtab->sh_addr);

  for (unsigned int n = 0; n < vsymtab->sh_size / sizeof(elf32_sym); n++, sym++) {
    if (ELF32_ST_TYPE(sym->st_info) != STT_FUNC) {
      continue;
    }
    if (addr >= sym->st_value && addr < (sym->st_value + sym->st_size)) {
      return (const char*)P2V(vstrtab->sh_addr) + sym->st_name;
    }
  }

  return NULL;
}

static void
print_trap_stacktrace (void) {
  kprintf("%s\n", "stacktrace:");

  unsigned int* esp;
  asm volatile("movl %%esp, %0" : "=r"(esp));

  esp += (sizeof(sig_context_t) / sizeof(unsigned int)) - 5;
  esp  = (unsigned int*)P2V((unsigned int)esp);
  for (unsigned int n = 1; n <= 32; n++) {
    kprintf(" %08x", *esp);
    esp++;
    if (!(n % 8)) {
      kprintf("%s", "\n");
    }
  }

  kprintf("%s", "Kernel stacktrace:\n");
  asm volatile("movl %%esp, %0" : "=r"(esp));

  esp += (sizeof(sig_context_t) / sizeof(unsigned int)) - 5;
  esp  = (unsigned int*)P2V((unsigned int)esp);

  for (unsigned int n = 0; n < 256; n++) {
    unsigned int addr = *esp;
    const char*  s    = elf_lookup_symbol(addr);
    if (s) {
      kprintf("<0x%08x> %s()\n", addr, s);
    }
    esp++;
  }
}

static retval_t
dump_trap_registers (unsigned int trap_num, sig_context_t* sc) {
  bool is_page_fault = trap_num == 14;

  if (is_page_fault) {
    unsigned int cr2;
    asm volatile("movl %%cr2, %0" : "=r"(cr2));
    kprintf(
      "%s at 0x%08x (%s) with errcode 0x%02x%s",
      traps_table[trap_num].name,
      cr2,
      sc->err & PAGE_FAULT_WRIT ? "writing" : "reading",
      sc->err,
      sc->err & PAGE_FAULT_USRMOD ? "\n" : " in kernel mode.\n"
    );
  } else {
    klogf_error("%s\n", traps_table[trap_num].name);
    if (traps_table[trap_num].errcode) {
      kprintf(": error code 0x%08x (0b%b)", sc->err, sc->err);
    }
    kprintf("%s", "\n");
  }
  kprintf(
    " cs: 0x%04x    eip: 0x%08x    efl: 0x%08x     ss: 0x%08x    esp: 0x%08x\n",
    sc->cs,
    sc->eip,
    sc->eflags,
    sc->og_ss,
    sc->og_esp
  );
  kprintf(
    "eax: 0x%08x    ebx: 0x%08x    ecx: 0x%08x    edx: 0x%08x\n",
    sc->eax,
    sc->ebx,
    sc->ecx,
    sc->edx
  );
  kprintf(
    "esi: 0x%08x    edi: 0x%08x    esp: 0x%08x    ebp: 0x%08x\n",
    sc->esi,
    sc->edi,
    sc->esp,
    sc->ebp
  );
  kprintf(
    " ds: 0x%04x     es: 0x%04x     fs: 0x%04x     gs: 0x%04x\n",
    sc->ds,
    sc->es,
    sc->fs,
    sc->gs
  );

  if (sc->cs == KERNEL_CS) {
    print_trap_stacktrace();
  }

  bool was_in_kernel_mode = sc->cs == KERNEL_CS;
  if (was_in_kernel_mode) {
    print_trap_stacktrace();
    return RET_FAIL;
  }

  return RET_OK;
}

void
trap_handle (unsigned int trap_num, sig_context_t sc) {
  traps_table[trap_num].handler(trap_num, &sc);
  sc.err = -sc.err;
  while (1);
}

void
trap_divide_error (unsigned int trap_num, sig_context_t* sc) {
  DUMP_REG_OR_FAIL(trap_num, sc);
}

void
trap_debug (unsigned int trap_num, sig_context_t* sc) {
  DUMP_REG_OR_FAIL(trap_num, sc);
}

void
trap_nmi_interrupt (unsigned int trap_num, sig_context_t* sc) {
  unsigned int error = inb(PS2_SYSCTRL_B);

  kprintf("NMI received: %d\n", error);
  switch (error) {
    case 0x80: kprintf("%s\n", "Parity check occurred.") break;
    default: kprintf("Unknown error: 0x%X\n", error); break;
  }

  DUMP_REG_OR_FAIL(trap_num, sc)
}

void
trap_breakpoint (unsigned int trap_num, sig_context_t* sc) {
  DUMP_REG_OR_FAIL(trap_num, sc);
}

void
trap_overflow (unsigned int trap_num, sig_context_t* sc) {
  DUMP_REG_OR_FAIL(trap_num, sc);
}

void
trap_bound (unsigned int trap_num, sig_context_t* sc) {
  DUMP_REG_OR_FAIL(trap_num, sc);
}

void
trap_invalid_opcode (unsigned int trap_num, sig_context_t* sc) {
  DUMP_REG_OR_FAIL(trap_num, sc);
}

void
trap_no_math_coprocessor (unsigned int trap_num, sig_context_t* sc) {
  DUMP_REG_OR_FAIL(trap_num, sc);
}

void
trap_double_fault (unsigned int trap_num, sig_context_t* sc) {
  DUMP_REG_OR_FAIL(trap_num, sc);
}

void
trap_coprocessor_segment_overrun (unsigned int trap_num, sig_context_t* sc) {
  DUMP_REG_OR_FAIL(trap_num, sc);
}

void
trap_invalid_tss (unsigned int trap_num, sig_context_t* sc) {
  DUMP_REG_OR_FAIL(trap_num, sc);
}

void
trap_segment_not_present (unsigned int trap_num, sig_context_t* sc) {
  DUMP_REG_OR_FAIL(trap_num, sc);
}

void
trap_stack_segment_fault (unsigned int trap_num, sig_context_t* sc) {
  DUMP_REG_OR_FAIL(trap_num, sc);
}

void
trap_general_protection (unsigned int trap_num, sig_context_t* sc) {
  DUMP_REG_OR_FAIL(trap_num, sc);
}

void
trap_page_fault (unsigned int trap_num, sig_context_t* sc) {}

void
trap_reserved (unsigned int trap_num, sig_context_t* sc) {
  DUMP_REG_OR_FAIL(trap_num, sc);
}

void
trap_floating_point_error (unsigned int trap_num, sig_context_t* sc) {
  DUMP_REG_OR_FAIL(trap_num, sc);
}

void
trap_alignment_check (unsigned int trap_num, sig_context_t* sc) {
  DUMP_REG_OR_FAIL(trap_num, sc);
}

void
trap_machine_check (unsigned int trap_num, sig_context_t* sc) {
  DUMP_REG_OR_FAIL(trap_num, sc);
}

void
trap_simd_fault (unsigned int trap_num, sig_context_t* sc) {
  DUMP_REG_OR_FAIL(trap_num, sc);
}
