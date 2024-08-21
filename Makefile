KERNEL_NAME := toulouse
KERNEL_VERSION := 0.0.1

ASM ?= as
EMU ?= qemu-system-x86_64
LD ?= ld
ELF_LD := ./deps/opt/cross/bin/i686-elf-ld
BARE_GCC := ./deps/opt/cross/bin/i686-elf-gcc
ZERO_DEV := /dev/zero

DEPS_DIR := deps
BIN_DIR := bin
BUILD_DIR := build

LINKER_CONF := linker.ld

BOOTLDR_SRC_DIR := boot
BOOTLDR_ENTRY := $(BOOTLDR_SRC_DIR)/main.s

BOOTLDR_TARGET_NAME := boot
BOOTLDR_OBJ := $(BUILD_DIR)/$(BOOTLDR_TARGET_NAME).o
BOOTLDR_BIN := $(BIN_DIR)/$(BOOTLDR_TARGET_NAME).bin

KERNEL_SRC_DIR := kernel
KERNEL_ENTRY := $(KERNEL_SRC_DIR)/main.s
KERNEL_TARGET_NAME := kernel
KERNEL_OBJ := $(BUILD_DIR)/$(KERNEL_TARGET_NAME).o
KERNEL_LINKED_OBJ := $(BUILD_DIR)/$(KERNEL_TARGET_NAME)_full.o
KERNEL_BIN := $(BIN_DIR)/$(KERNEL_TARGET_NAME).bin

KERNEL_ASM_DEBUG_FLAGS := -g
KERNEL_GCC_DEBUG_FLAGS := -O0
KERNEL_GCC_FLAGS := -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce \
-fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter \
-nostdlib -nostartfiles -nodefaultlibs -Wall -Iinc

KERNEL_C_ENTRY := $(KERNEL_SRC_DIR)/main.c
KERNEL_C_ENTRY_OBJ := $(BUILD_DIR)/main.o
KERNEL_OBJS := $(KERNEL_OBJ) $(KERNEL_C_ENTRY_OBJ)
KERNEL_INCLUDES := $(KERNEL_SRC_DIR)

OS_BIN := $(BIN_DIR)/$(KERNEL_NAME)-$(KERNEL_VERSION).bin

dirs:
	mkdir -p $(BUILD_DIR) $(BIN_DIR)

$(BOOTLDR_OBJ): dirs
	$(ASM) $(BOOTLDR_ENTRY) -o $(BOOTLDR_OBJ)

$(KERNEL_OBJ): dirs
	$(ASM) --32 $(KERNEL_ENTRY) -o $(KERNEL_OBJ)

$(KERNEL_C_ENTRY_OBJ):
	$(BARE_GCC) $(KERNEL_GCC_FLAGS) -I$(KERNEL_INCLUDES) -std=gnu99 -c $(KERNEL_C_ENTRY) -o $(KERNEL_C_ENTRY_OBJ)

bootloader: $(BOOTLDR_OBJ)
	$(LD) -T $(BOOTLDR_SRC_DIR)/$(LINKER_CONF) -o $(BOOTLDR_BIN) $(BOOTLDR_OBJ)

kernel: $(KERNEL_OBJS)
	$(ELF_LD) -relocatable $(KERNEL_OBJS) -o $(KERNEL_LINKED_OBJ)
	$(BARE_GCC) $(KERNEL_GCC_FLAGS) -T $(LINKER_CONF) $(KERNEL_LINKED_OBJ) -o $(KERNEL_BIN)

all: bootloader kernel
	dd if=$(BOOTLDR_BIN) >> $(OS_BIN)
	dd if=$(KERNEL_BIN) >> $(OS_BIN)
	# Pad the image and align at 512 bytes
	dd if=$(ZERO_DEV) bs=512 count=100 >> $(OS_BIN)

# Exit Qemu focus trap with ctrl+alt+g
run:
	$(EMU) -hda $(OS_BIN)

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

clean_deps:
	rm -rf $(DEPS_DIR)

.PHONY: all dirs bootloader kernel run clean clean_deps
