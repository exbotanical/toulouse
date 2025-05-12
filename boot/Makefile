ROOTPATH ?= $(shell readlink -f ..)
include $(ROOTPATH)/Makefile.config

.PHONY: all clean dirs
.DELETE_ON_ERROR:

STAGE1_OBJ    := $(BUILD_DIRNAME)/stage1.o
STAGE2_OBJ    := $(BUILD_DIRNAME)/stage2.o
LOADER_OBJ    := $(BUILD_DIRNAME)/loader.o
BOOT_TARGET   := $(BIN_DIRNAME)/bootblock

EXT_OBJS := ../drivers/build/dev/block/ata.o
EXT_OBJS += $(patsubst ../lib/src/%.c,../lib/build/%.o,$(shell find -L ../lib/src -type f -name "*.c"))

# -mkernel?
CFLAGS        := -m32 -static -nostdinc -fno-builtin -fno-omit-frame-pointer \
	-fno-pic -ffreestanding -O3
CFLAGS        += $(C_STRICTMODE_FLAGS)
CFLAGS        +=  -I../$(INC_DIRNAME)

LDFLAGS       := -nostdlib -O3
LDFLAGS       += -m $(shell $(LD) -V | grep elf_i386 2>/dev/null | head -n 1)

ASFLAGS       := --32 -divide

all: dirs $(BOOT_TARGET)

clean:
	rm -rf $(BUILD_DIRNAME)
	rm -rf $(BIN_DIRNAME)

dirs:
	@mkdir -p $(BUILD_DIRNAME) $(BIN_DIRNAME)

$(EXT_OBJS):
	$(MAKE) -C ../{lib,drivers} all

$(STAGE1_OBJ): src/stage1/main.s
	$(CC) $(CFLAGS) -c $< -o $@

$(STAGE2_OBJ): src/stage2/main.s
	$(CC) $(CFLAGS) -c $< -o $@

$(LOADER_OBJ): src/main.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BOOT_TARGET): $(STAGE1_OBJ) $(STAGE2_OBJ) $(LOADER_OBJ) $(EXT_OBJS)
	$(LD) $(LDFLAGS) -T linker.ld $^ -o $@
