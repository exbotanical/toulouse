.DELETE_ON_ERROR:

KERNEL_NAME := toulouse
KERNEL_VERSION := 0.0.1


ASM ?= as
EMU ?= qemu-system-x86_64
LD ?= ld
ELF_LD := ./deps/opt/cross/bin/i686-elf-ld
CC := gcc
OBJCOPY = objcopy
OBJDUMP = objdump
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
# CFLAGS := -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce \
# -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter \
# -nostdlib -nostartfiles -nodefaultlibs -Wall -Iinc

CFLAGS = -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -m32 -Werror -fno-omit-frame-pointer
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)

LDFLAGS += -m $(shell $(LD) -V | grep elf_i386 2>/dev/null | head -n 1)
ASFLAGS = -m32 -gdwarf-2 -Wa,-divide

KERNEL_C_ENTRY := $(KERNEL_SRC_DIR)/main.c
KERNEL_C_ENTRY_OBJ := $(BUILD_DIR)/main.o
KERNEL_OBJS := $(KERNEL_OBJ) $(KERNEL_C_ENTRY_OBJ)
KERNEL_INCLUDES := $(KERNEL_SRC_DIR)

OS_BIN := $(BIN_DIR)/$(KERNEL_NAME)-$(KERNEL_VERSION).bin

dirs:
	mkdir -p $(BUILD_DIR) $(BIN_DIR)

$(BOOTLDR_OBJ): dirs
	$(ASM) $(BOOTLDR_ENTRY) -o $(BOOTLDR_OBJ)

# $(KERNEL_OBJ): dirs
# 	$(ASM) --32 $(KERNEL_ENTRY) -o $(KERNEL_OBJ)

$(KERNEL_C_ENTRY_OBJ):
	$(CC) $(CFLAGS) -I$(KERNEL_INCLUDES) -std=gnu99 -c $(KERNEL_C_ENTRY) -o $(KERNEL_C_ENTRY_OBJ)

# bootloader: $(BOOTLDR_OBJ)
# 	$(LD) -T $(BOOTLDR_SRC_DIR)/$(LINKER_CONF) -o $(BOOTLDR_BIN) $(BOOTLDR_OBJ)

t.img: bootblock kernel
	dd if=/dev/zero of=t.img count=10000
	dd if=bin/bootblock of=t.img conv=notrunc
# TODO: placement
	# dd if=bin/kernel of=t.img seek=10 conv=notrunc

bootblock: boot/main.s boot/main.c dirs
	$(CC) $(CFLAGS) -fno-pic -O -nostdinc -I./boot -c boot/main.c -o build/kernel.o
	$(CC) $(CFLAGS) -fno-pic -nostdinc -I./boot -c boot/main.s -o build/main.o
	$(LD) $(LDFLAGS) -T $(BOOTLDR_SRC_DIR)/$(LINKER_CONF) -o build/bootblock.o build/main.o build/kernel.o
	$(OBJCOPY) -S -O binary -j .text build/bootblock.o bin/bootblock

# bootblock: dirs
# 	$(CC) $(CFLAGS) -fno-pic -O -nostdinc -I./boot -c boot/main.c -o build/kernel.o
# 	$(CC) $(CFLAGS) -fno-pic -nostdinc -I./boot -c boot/stage1/main.s -o build/stage1.o
# 	$(CC) $(CFLAGS) -fno-pic -nostdinc -I./boot -c boot/stage2/main.s -o build/stage2.o
# 	$(LD) $(LDFLAGS) -T $(BOOTLDR_SRC_DIR)/$(LINKER_CONF) -o build/bootblock.o build/stage1.o build/stage2.o build/kernel.o
# 	$(OBJCOPY) -S -O binary -j .text build/bootblock.o bin/bootblock


# build/kernel.o: dirs
# 	$(CC) $(ASFLAGS) -c -o build/kernel.o kernel/main.s

# kernel: $(OBJS) build/kernel.o linker.ld
# 	$(LD) $(LDFLAGS) -T linker.ld -o bin/kernel build/kernel.o $(OBJS) -b binary

# kernel: $(KERNEL_OBJS)
# 	$(ELF_LD) -relocatable $(KERNEL_OBJS) -o $(KERNEL_LINKED_OBJ)
# 	$(CC) $(CFLAGS) -T $(LINKER_CONF) $(KERNEL_LINKED_OBJ) -o $(KERNEL_BIN)

all: bootloader kernel
	dd if=$(BOOTLDR_BIN) >> $(OS_BIN)
	dd if=$(KERNEL_BIN) >> $(OS_BIN)
	# Pad the image and align at 512 bytes
	dd if=$(ZERO_DEV) bs=512 count=100 >> $(OS_BIN)

# Exit Qemu focus trap with ctrl+alt+g
test:
	$(MAKE) clean
	$(MAKE) t.img
	$(EMU) -drive file=t.img,index=0,media=disk,format=raw

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

clean_deps:
	rm -rf $(DEPS_DIR)

.PHONY: all dirs bootloader kernel test clean clean_deps
