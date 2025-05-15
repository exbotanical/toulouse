ROOTPATH ?= $(shell readlink -f ..)
include $(ROOTPATH)/Makefile.config

.PHONY: all clean dirs unit_test unit_test_dev
.DELETE_ON_ERROR:

CSRCS            := $(shell find -L $(SRC_DIRNAME) -type f -name "*.c")
OBJS             := $(patsubst $(SRC_DIRNAME)/%.c,$(BUILD_DIRNAME)/%.o,$(CSRCS))
EXT_OBJS         := $(patsubst ../lib/src/%.c,../lib/build/%.o,$(shell find -L ../lib/src -type f -name "*.c"))
TEST_SRCS        := $(shell find -L $(TEST_DIRNAME) -type f -name "*.c")
TEST_DEPS        := $(wildcard ../$(DEPS_DIRNAME)/libtap/*.c)
TEST_WATCH_FILES := $(shell find ../$(INC_DIRNAME) $(SRC_DIRNAME) $(TEST_DIRNAME) -type f \( -name "*.c" -o -name "*.h" \))

CFLAGS           := -m32 -std=gnu99 -fno-strict-aliasing -nostdlib -ffreestanding -c -O3
CFLAGS           += $(C_STRICTMODE_FLAGS)
CFLAGS           += -I../$(INC_DIRNAME)

LDFLAGS          := -nostdlib -O3
LDFLAGS          += -m $(shell $(LD) -V | grep elf_i386 2>/dev/null | head -n 1)

all: dirs $(OBJS)

clean:
	rm -rf $(BUILD_DIRNAME)
	rm -rf $(UT_TARGET)

dirs:
	@mkdir -p $(BUILD_DIRNAME)

unit_test: $(OBJS) $(EXT_OBJS)
	gcc -m32 $(TEST_SRCS) $(TEST_DEPS) $(OBJS) $(EXT_OBJS) -I../$(DEPS_DIRNAME) -I../$(INC_DIRNAME) -I./$(TEST_DIRNAME) -o $(UT_TARGET)
	./$(UT_TARGET)
	$(MAKE) clean

unit_test_dev:
	ls $(TEST_WATCH_FILES) | entr -s 'make -s unit_test'

$(EXT_OBJS):
	$(MAKE) -C ../lib all

$(OBJS): $(BUILD_DIRNAME)/%.o : $(SRC_DIRNAME)/%.c $(EXT_OBJS)
	@mkdir -p $(shell dirname $@)
	$(CC) $(CFLAGS) -o $@ $<
