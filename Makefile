CC=i686-elf-gcc
ASSEMBLER=nasm
EMULATOR=qemu-system-x86_64

BOOTLOADER_FLAGS=-f bin 
KERNEL_FLAGS=-f elf -g
CC_FLAGS=-g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc
EMU_FLAGS=-hda

BOOTLOADER_SRC=src/boot/boot.asm
BOOTLOADER_BIN=bin/boot.bin

KERNEL_SRC=src/kernel.c
KERNEL_ASM_SRC=src/kernel.asm
KERNEL_ASM_OBJ=build/kernel.asm.o
KERNEL_OBJ=build/kernel.o
KERNEL_BIN=bin/kernel.bin

FULL_KERNEL_OBJ=build/kernelfull.o
OS_BIN=bin/os.bin

LINKER_SRC=src/linker.ld

BUILD_DEPS=$(KERNEL_ASM_OBJ) $(KERNEL_OBJ)
INCLUDES= -I./src

# build
all: clean $(BOOTLOADER_BIN) $(KERNEL_BIN)
	dd if=$(BOOTLOADER_BIN) >> $(OS_BIN)
	dd if=$(KERNEL_BIN) >> $(OS_BIN)
	dd if=/dev/zero bs=512 count=100 >> $(OS_BIN)

# emulate bootloader
emu: all
	$(EMULATOR) $(EMU_FLAGS) $(OS_BIN)

# write bootloader to USB
usb: all
	sudo dd if=./$(BOOTLOADER_BIN) of=/dev/sdb

# build bootloader binary
$(BOOTLOADER_BIN): $(BOOTLOADER_SRC)
	$(ASSEMBLER) $(BOOTLOADER_FLAGS) $(BOOTLOADER_SRC) -o $(BOOTLOADER_BIN)

# build kernel object file
$(KERNEL_ASM_OBJ): $(KERNEL_ASM_SRC)
	$(ASSEMBLER) $(KERNEL_FLAGS) $(KERNEL_ASM_SRC) -o $(KERNEL_ASM_OBJ)
 
# build final object file
$(KERNEL_BIN): $(BUILD_DEPS)
	i686-elf-ld -g -relocatable $(BUILD_DEPS) -o $(FULL_KERNEL_OBJ)
	$(CC) $(CC_FLAGS) -T $(LINKER_SRC) -o $(KERNEL_BIN) -ffreestanding -O0 -nostdlib $(FULL_KERNEL_OBJ)

# build kernel source file
$(KERNEL_OBJ): $(KERNEL_SRC)
	$(CC) $(INCLUDES) $(CC_FLAGS) -std=gnu99 -c $(KERNEL_SRC) -o $(KERNEL_OBJ)

clean:
	rm  -f ./bin/*.bin 
	rm -f ./build/*.o

.PHONY: run clean
