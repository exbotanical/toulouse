ROOTPATH ?= $(shell readlink -f ..)
include $(ROOTPATH)/Makefile.config

.PHONY: all dirs clean
.DELETE_ON_ERROR:

BUILD_DIR 	 := build
BIN_DIR			 := bin
INC_DIR 		 := .

STAGE1_OBJ 	 := $(BUILD_DIR)/stage1.o
STAGE2_OBJ 	 := $(BUILD_DIR)/stage2.o
LOADER_OBJ 	 := $(BUILD_DIR)/loader.o
BOOT_TARGET  := $(BIN_DIR)/bootblock

C_STRICTMODE_FLAGS := -Wall -Werror -Wextra -Wno-missing-field-initializers \
	-Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition \
	-Wno-unused-parameter -Wno-unused-function -Wno-unused-value -pedantic

# -mkernel?
CFLAGS	:= -m32 -static -nostdinc -fno-builtin -fno-omit-frame-pointer \
	-fno-pic -ffreestanding -O3
CFLAGS 	+= $(C_STRICTMODE_FLAGS)
CFLAGS 	+= -I$(INC_DIR)

LDFLAGS := -nostdlib -O3
LDFLAGS += -m $(shell $(LD) -V | grep elf_i386 2>/dev/null | head -n 1)

ASFLAGS := -m32 -Wa,-divide

$(STAGE1_OBJ): src/stage1/main.s
	$(CC) $(CFLAGS) -c $< -o $@

$(STAGE2_OBJ): src/stage2/main.s
	$(CC) $(CFLAGS) -c $< -o $@

$(LOADER_OBJ): src/main.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BOOT_TARGET): $(STAGE1_OBJ) $(STAGE2_OBJ) $(LOADER_OBJ)
	$(LD) $(LDFLAGS) -T linker.ld $^ -o $@

all: dirs $(BOOT_TARGET)

clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(BIN_DIR)

dirs:
	@mkdir -p $(BUILD_DIR) $(BIN_DIR)
