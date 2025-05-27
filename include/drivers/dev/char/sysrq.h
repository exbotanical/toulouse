#ifndef SYSRQ_H
#define SYSRQ_H

/* SysRq ops - Alt+SysRq+one of the below keys */
#define SYSRQ_STACK  0x00000001 /* 'l' -> stack backtrace */
#define SYSRQ_MEMORY 0x00000002 /* 'm' -> memory information */
#define SYSRQ_TASKS  0x00000004 /* 't' -> task list */
#define SYSRQ_UNDEF  0x80000000 /* Undefined operation */

void sysrq(int);

#endif /* SYSRQ_H */
