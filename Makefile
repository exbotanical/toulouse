ASM ?= nasm
EMU ?= qemu-system-x86_64
ELF_LD ?= i686-elf-ld
ELF_GCC ?= i686-elf-gcc

ASM_BIN_FLAGS := -f bin
LD_FLAGS := -g -relocatable
ELF_GCC_FLAGS := -ffreestanding -O0 -nostdlib
KERNEL_GCC_FLAGS := -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -Wall -O0 -Iinc

BUILD_OUTDIR := build
BIN_OUTDIR := bin

BOOT_SRC_DIR := boot
BOOT_SRC_FILE := $(BOOT_SRC_DIR)/boot.asm
BOOT_TARGET := $(BIN_OUTDIR)/boot.bin

KERNEL_SRC_DIR := kernel
KERNEL_SRC_FILE := $(KERNEL_SRC_DIR)/kernel.asm
KERNEL_TARGET := $(BIN_OUTDIR)/kernel.bin

LINKER_CONF := linker.ld

KERNEL_DEBUG ?= -g

OS_TARGET := $(BIN_OUTDIR)/os.bin

LINKED_OBJ_TARGET := $(BUILD_OUTDIR)/kernelld.o
KERNEL_INIT_OBJ := $(BUILD_OUTDIR)/kernel.asm.o
KERNEL_OBJ := $(BUILD_OUTDIR)/kernel.o
OBJ_FILES = $(patsubst $(KERNEL_SRC_DIR)/%.c, $(BUILD_OUTDIR)/%.o, $(wildcard $(KERNEL_SRC_DIR)/*.c))

INCLUDES := $(KERNEL_SRC_DIR)

all: $(BOOT_TARGET) $(KERNEL_TARGET) $(OBJ_FILES)
	dd if=$(BOOT_TARGET) >> $(OS_TARGET)
	dd if=$(KERNEL_TARGET) >> $(OS_TARGET)
	dd if=/dev/zero bs=512 count=100 >> $(OS_TARGET)

$(KERNEL_TARGET): $(OBJ_FILES)
	$(ELF_LD) $(LD_FLAGS) $(OBJ_FILES) -o $(LINKED_OBJ_TARGET)
	$(ELF_GCC) $(KERNEL_GCC_FLAGS) -T $(LINKER_CONF) -o $(KERNEL_TARGET) $(ELF_GCC_FLAGS) $(LINKED_OBJ_TARGET)

$(BOOT_TARGET): setup $(BOOT_SRC_FILE)
	$(ASM) $(ASM_BIN_FLAGS) $(BOOT_SRC_FILE) -o $(BOOT_TARGET)

$(KERNEL_INIT_OBJ): $(KERNEL_SRC_FILE)
	$(ASM) -f elf $(KERNEL_DEBUG) $(KERNEL_SRC_FILE) -o $(KERNEL_INIT_OBJ)

$(OBJ_FILES): $(BUILD_OUTDIR)/%.o: $(KERNEL_SRC_DIR)/%.c
	$(ELF_GCC) -I$(INCLUDES) $(KERNEL_GCC_FLAGS) -std=gnu99 -c $< -o $@

setup:
	mkdir -p $(BIN_OUTDIR) $(BUILD_OUTDIR)

run:
	$(EMU) -hda $(OS_TARGET)

clean:
	rm -rf $(BIN_OUTDIR) $(BUILD_OUTDIR) **/*.o

.PHONY: all setup run clean
