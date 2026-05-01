CC      = gcc
AR      = ar
CFLAGS  = -Wall -Wextra -Wconversion -Wsign-conversion -O0 -ggdb3 \
          -fno-omit-frame-pointer -fno-optimize-sibling-calls \
          -fasynchronous-unwind-tables -fPIC -Iinclude
LDFLAGS = -pthread

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

.PHONY: all shared static tests clean rebuild

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

rebuild: clean all