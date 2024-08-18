ASM ?= as
EMU ?= qemu-system-x86_64
LD ?= ld

DEPS_DIR := deps
BIN_DIR := bin
BUILD_DIR := build

BOOTLDR_SRC_DIR := boot

BOOTLDR_ENTRY_FILENAME := main
BOOTLDR_BIN := $(BIN_DIR)/$(BOOTLDR_ENTRY_FILENAME).bin

LINKER_CONF := linker.ld

dirs:
	mkdir -p $(BUILD_DIR) $(BIN_DIR)

bootloader: dirs
	$(ASM) $(BOOTLDR_SRC_DIR)/$(BOOTLDR_ENTRY_FILENAME).s -o $(BUILD_DIR)/$(BOOTLDR_ENTRY_FILENAME).o
	$(LD) --oformat=binary -T $(LINKER_CONF) -o $(BOOTLDR_BIN) $(BUILD_DIR)/$(BOOTLDR_ENTRY_FILENAME).o

# Exit Qemu focus trap with ctrl+alt+g
run:
	$(EMU) -hda $(BOOTLDR_BIN)

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

clean_deps:
	rm -rf $(DEPS_DIR)

.PHONY: dirs, bootloader, run, clean, clean_deps
