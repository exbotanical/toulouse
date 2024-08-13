ASM ?= as
EMU ?= qemu-system-x86_64
LD ?= ld

DEPS_DIR := deps
OS_TARGET := boot.bin

bootloader:
	$(ASM) boot/boot.s -o boot.o
	$(LD) --oformat=binary -T linker.ld -o boot.bin boot.o

# Exit Qemu focus trap with ctrl+alt+g
.PHONY: run
run:
	$(EMU) -hda $(OS_TARGET)

.PHONY: clean_deps; clean_deps:
	rm -rf $(DEPS_DIR)
