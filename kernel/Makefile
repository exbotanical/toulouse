ROOTPATH ?= $(shell readlink -f ..)
include $(ROOTPATH)/Makefile.config

.PHONY: all clean dirs linker unit_test unit_test_dev
.DELETE_ON_ERROR:

LD_TARGET        := $(shell mktemp)

CSRCS            := $(shell find -L $(SRC_DIRNAME) -type f -name "*.c")
ASM_SRCS         := $(shell find -L $(SRC_DIRNAME) -type f -name "*.S")

OBJS             := $(patsubst $(SRC_DIRNAME)/%.c, $(BUILD_DIRNAME)/%.o, $(CSRCS))
ASM_OBJS         := $(patsubst $(SRC_DIRNAME)/%.S, $(BUILD_DIRNAME)/%.asm.o, $(ASM_SRCS))

EXT_OBJS         := $(patsubst ../drivers/src/%.c,../drivers/build/%.o,$(shell find -L ../drivers/src -type f -name "*.c"))
EXT_OBJS         += $(patsubst ../lib/src/%.c,../lib/build/%.o,$(shell find -L ../lib/src -type f -name "*.c"))

TEST_DEPS        := $(wildcard ../$(DEPS_DIRNAME)/libtap/*.c)
TEST_SRCS        := $(shell find -L $(TEST_DIRNAME) -type f -name "*.c")
TEST_ASM_OBJS    := $(filter-out $(BUILD_DIRNAME)/loader.asm.o, $(ASM_OBJS))
TEST_WATCH_FILES := $(shell find ../$(INC_DIRNAME) $(SRC_DIRNAME) $(TEST_DIRNAME) -type f \( -name "*.c" -o -name "*.h" \))

CFLAGS           := -m32 -std=gnu99 -fno-strict-aliasing -nostdlib -ffreestanding \
	-c -mno-sse
# Why no SSE - see: https://forum.osdev.org/viewtopic.php?p=234155
# TODO: -fstack-protector-all (need to link)

CFLAGS           += $(C_STRICTMODE_FLAGS)
CFLAGS           += -I../$(INC_DIRNAME)

LDFLAGS          := -nostdlib
LDFLAGS          += -m $(shell $(LD) -V | grep elf_i386 2>/dev/null | head -n 1)

ifdef DEBUG
	CFLAGS += -O0 -g -DDEBUG
	LDFLAGS += -O0 -g
else
	CFLAGS += -O3
	LDFLAGS += -O3
endif

$(EXT_OBJS):
	$(MAKE) -C ../lib all
	$(MAKE) -C ../drivers all

$(OBJS): $(BUILD_DIRNAME)/%.o : $(SRC_DIRNAME)/%.c
	@mkdir -p $(shell dirname $@)
	$(CC) $(CFLAGS) -o $@ $<

$(ASM_OBJS): $(BUILD_DIRNAME)/%.asm.o : $(SRC_DIRNAME)/%.S
	@mkdir -p $(shell dirname $@)
	$(CC) $(CFLAGS) -o $@ $<

all: dirs linker $(OBJS) $(EXT_OBJS) $(ASM_OBJS)
	$(LD) $(LDFLAGS) -T $(LD_TARGET) $(ASM_OBJS) $(OBJS) $(EXT_OBJS) -o bin/kernel

clean:
	rm -rf $(BUILD_DIRNAME) $(BIN_DIRNAME)
	rm -rf $(UT_TARGET) $(LD_TARGET)

dirs:
	@mkdir -p $(BUILD_DIRNAME) $(BIN_DIRNAME)

linker:
	cpp -P linker.ld $(INC) >| $(LD_TARGET)

unit_test: $(OBJS) $(EXT_OBJS) $(TEST_ASM_OBJS)
	gcc -m32 $(TEST_SRCS) $(TEST_DEPS) $^ -I../{$(DEPS_DIRNAME),$(INC_DIRNAME)} -o $(UT_TARGET)
	./$(UT_TARGET)
	$(MAKE) clean

unit_test_dev:
	ls $(TEST_WATCH_FILES) | entr -s 'make -s unit_test'
