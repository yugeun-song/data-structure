CC = gcc
AR = ar

BUILD_TYPE ?= debug

STD_FLAGS  = -std=c99 -pedantic
WARN_FLAGS = -Wall -Wextra -Wconversion -Wsign-conversion

ifeq ($(BUILD_TYPE), release)
    OPT_FLAGS = -O2 -DNDEBUG
    DEBUG_LDFLAGS =
else ifeq ($(BUILD_TYPE), debug)
    OPT_FLAGS = -O0 -ggdb3 \
                -fno-omit-frame-pointer -fno-optimize-sibling-calls \
                -fno-builtin \
                -fasynchronous-unwind-tables \
                -pg
    DEBUG_LDFLAGS = -pg -rdynamic
else
    $(error Unknown BUILD_TYPE '$(BUILD_TYPE)' -- expected 'debug' or 'release')
endif

CFLAGS  = $(STD_FLAGS) $(WARN_FLAGS) $(OPT_FLAGS) -fPIC -Iinclude
LDFLAGS = $(DEBUG_LDFLAGS)

LIB_NAME = ds

INC_DIR  = include
SRC_DIR  = src
TEST_DIR = tests
BIN_DIR  = bin
OBJ_DIR  = $(BIN_DIR)/obj

LIB_SRCS = $(wildcard $(SRC_DIR)/*.c)
LIB_OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(LIB_SRCS))

SHARED_LIB = $(BIN_DIR)/lib$(LIB_NAME).so
STATIC_LIB = $(BIN_DIR)/lib$(LIB_NAME).a

TEST_SRCS = $(wildcard $(TEST_DIR)/*.c)
TEST_BINS = $(patsubst $(TEST_DIR)/%.c, $(BIN_DIR)/test/%, $(TEST_SRCS))

.PHONY: all shared static tests clean distclean rebuild compile-db

all: shared static

shared: $(SHARED_LIB)

static: $(STATIC_LIB)

tests: $(TEST_BINS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(SHARED_LIB): $(LIB_OBJS)
	@mkdir -p $(dir $@)
	$(CC) -shared -o $@ $^ $(LDFLAGS)

$(STATIC_LIB): $(LIB_OBJS)
	@mkdir -p $(dir $@)
	$(AR) rcs $@ $^

$(BIN_DIR)/test/%: $(TEST_DIR)/%.c $(STATIC_LIB)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< $(STATIC_LIB) -o $@ $(LDFLAGS)

clean:
	rm -rf $(BIN_DIR)

distclean: clean
	rm -rf build out .cache
	rm -f compile_commands.json
	rm -f gmon.out perf.data perf.data.old
	rm -f callgrind.out.* cachegrind.out.* massif.out.* helgrind.out.* drd.out.*
	rm -f valgrind.log valgrind*.log valgrind*.xml
	rm -f *.gcda *.gcno *.profraw *.profdata
	rm -rf uftrace.data
	rm -f strace.out strace.log *.strace ltrace.out ltrace.log *.ltrace
	rm -f .gdb_history .lldb_history gdb.txt
	rm -f peda-session-*.txt .gef-history .pwndbg_history
	rm -f core core.* vgcore.*
	rm -f asan.log.* ubsan.log.* tsan.log.* msan.log.*
	rm -f *.dwo *.dwp *.debug
	rm -f qemu.log trace-events-all

rebuild: clean all

compile-db:
	cmake -B build -G Ninja
	ln -sf build/compile_commands.json compile_commands.json