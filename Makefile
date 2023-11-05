ASM ?= nasm
EMU ?= qemu-system-x86_64

ASM_BIN_FLAGS := -f bin

BOOT_DIR := boot
BOOT_FILE := boot.asm
BOOT_TARGET := boot.bin

BUILD_OUTDIR := build
BIN_OUTDIR := bin

setup:
	mkdir -p $(BIN_OUTDIR) $(BUILD_OUTDIR)

bootloader: setup
	$(ASM) $(ASM_BIN_FLAGS) $(BOOT_DIR)/$(BOOT_FILE) -o $(BIN_OUTDIR)/$(BOOT_TARGET)

run: bootloader
	$(EMU) -hda $(BIN_OUTDIR)/$(BOOT_TARGET)

clean:
	rm -rf $(BIN_OUTDIR) $(BUILD_OUTDIR)

.PHONY: setup bootloader run clean
