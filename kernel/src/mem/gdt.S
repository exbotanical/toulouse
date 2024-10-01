.code32

#define ASM_SOURCE 1

#include "mem/segments.h"

.global gdt_load; gdt_load:
  movl    0x4(%esp), %eax
  lgdt    (%eax)
  movw    $KERNEL_DS, %ax
  movw    %ax, %ds
  movw    %ax, %es
  movw    %ax, %fs
  movw    %ax, %gs
  movw    %ax, %ss
  ljmp    $KERNEL_CS, $done
done:
  ret
