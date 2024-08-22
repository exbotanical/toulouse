
__attribute__((section(".kernel"))) void
kernel_startx (void) {
  asm("mov $0xB8000, %edx\n"
      "movb $'X', %al\n"
      "mov $0x0F, %ah\n"
      "mov %ax, (%edx)\n"
      "inc %edx");
  asm("movb $'X', %al\n"
      "mov $0x0F, %ah\n"
      "mov %ax, (%edx)\n"
      "inc %edx");
  asm("movb $'X', %al\n"
      "mov $0x0F, %ah\n"
      "mov %ax, (%edx)\n"
      "inc %edx");
  asm("movb $'X', %al\n"
      "mov $0x0F, %ah\n"
      "mov %ax, (%edx)\n"
      "inc %edx");

  unsigned short int* vga = (unsigned short int*)(0xB80000);

  vga[100]                = 0x0341;
  vga[101]                = 0x0341;
  vga[102]                = 0x0341;
  vga[103]                = 0x0341;
  vga[101]                = ('h' & 0xff) | 0x0700;
  while (1) {
    vga[100] = 0x0341;
  }
}
