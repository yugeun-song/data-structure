# data-structure

A collection of generic data structures implemented in pure C with
small amounts of inline assembly, designed for high-performance
multithreaded use. Organization mirrors Linux kernel conventions while
exposing an STL-container-like surface.

## Directory Structure

```
.
├── bin                       Build output (generated)
├── include                   Public headers
├── src                       Library implementation
├── tests                     Test programs and header-only test framework
├── CMakeLists.txt            CMake build configuration
├── CMakePresets.json         CMake presets for WSL environments
├── CMakeSettings.json        Visual Studio CMake integration
├── Makefile                  GNU Make build configuration
└── README.md
```

Library sources in `src/` compile into `bin/libds.so` (shared) and
`bin/libds.a` (static). Public API headers live in `include/`. Test
programs in `tests/` compile into `bin/test/<name>` and link against
the static library.

## Build

### Prerequisites

- GCC or Clang
- GNU Make or CMake (>= 3.10)

### Build Modes

| Mode    | Optimization | Debug symbols + frame pointers | Profiling hooks | NDEBUG |
|---------|--------------|--------------------------------|-----------------|--------|
| debug   | -O0          | yes                            | -pg, -rdynamic  | no     |
| release | -O2          | no                             | none            | yes    |

Debug is the default. Debug builds carry extensive instrumentation so
that gdb, perf, gprof, uftrace, ltrace, and strace can all extract
detailed runtime data without recompilation.

### Compiler

Both GCC and Clang are supported across both build systems and both
build modes. The same flag set is applied to either compiler; `-pg`
mcount instrumentation is fully effective under GCC and partially
effective under Clang (Clang treats it more conservatively but does
not reject it).

### Make

Make uses `gcc` by default. Override via `CC`:

```sh
make                                # debug build (gcc)
make BUILD_TYPE=release             # release build (gcc)
make CC=clang                       # debug build (clang)
make CC=clang BUILD_TYPE=release    # release build (clang)
make tests                          # build test programs
make clean                          # remove bin/ (make build outputs)
make distclean                      # also remove every generated artifact:
                                    #   cmake build/, out/, clangd .cache/
                                    #   gmon.out, perf.data
                                    #   valgrind: callgrind/cachegrind/massif/
                                    #     helgrind/drd outputs, valgrind*.log/.xml
                                    #   *.gcda/*.gcno (gcov), *.profraw/*.profdata
                                    #     (clang coverage)
                                    #   uftrace.data/, strace/ltrace logs
                                    #   gdb/lldb history, peda/gef/pwndbg sessions
                                    #   core dumps (core, core.*, vgcore.*)
                                    #   asan/ubsan/tsan/msan logs
                                    #   split DWARF (*.dwo, *.dwp, *.debug)
                                    #   qemu.log, trace-events-all
```

### CMake

CMake auto-detects the compiler. Override via `CMAKE_C_COMPILER`:

```sh
cmake -B build -G Ninja                                                  # debug, autodetect
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release                       # release, autodetect
cmake -B build -G Ninja -DCMAKE_C_COMPILER=clang                         # debug, clang
cmake -B build -G Ninja -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Release
cmake --build build
rm -rf build                                                             # full cmake clean
```

A preset for WSL environments is provided:

```sh
cmake --preset wsl-debug
cmake --build out/build/wsl-debug
```

### Editor Integration (clangd / LSP)

`clangd` (used by Neovim, VS Code, and similar editors) reads
`compile_commands.json` to resolve include paths and compile flags.
CMake emits this file automatically; symlink it to the project root so
editors find it from any source file:

```sh
make compile-db
```

This is equivalent to:

```sh
cmake -B build -G Ninja
ln -sf build/compile_commands.json compile_commands.json
```

The symlink is gitignored. Re-run after adding sources or changing
build flags. `make distclean` removes both the build directory and the
symlink; regenerate with `make compile-db`.

### Compiler Flags

Common to both modes:

```
-std=c99 -pedantic
-Wall -Wextra -Wconversion -Wsign-conversion -fPIC
```

The C standard is pinned to ISO C99; GNU/POSIX extensions are disabled
(`CMAKE_C_EXTENSIONS OFF` on the CMake side, `-std=c99 -pedantic` on
the Make side). C11+ features are not used in this project.

Debug-mode additions:

```
-O0 -ggdb3
-fno-omit-frame-pointer -fno-optimize-sibling-calls
-fno-builtin
-fasynchronous-unwind-tables
-pg
```

Debug-mode linker flags:

```
-pg -rdynamic
```

Release-mode additions:

```
-O2 -DNDEBUG
```

### Profiling and Tracing (Debug Mode)

Debug builds compile and link with hooks that work with the following
tools without further recompilation:

| Tool       | Hook used                                                           |
|------------|---------------------------------------------------------------------|
| gdb / lldb | `-O0`, `-ggdb3`, frame pointers                                     |
| perf       | `-ggdb3`, frame pointers, `-fasynchronous-unwind-tables`            |
| gprof      | `-pg` (mcount instrumentation, generates gmon.out at exit)          |
| uftrace    | `-pg` (mcount-based runtime tracing)                                |
| valgrind   | `-O0`, `-ggdb3`, frame pointers (memcheck / callgrind / massif etc.)|
| ltrace     | `-fno-builtin` keeps libc calls observable as actual call sites     |
| strace     | none required (operates at syscall level)                           |
| backtrace  | `-rdynamic` exports symbols for dl_iterate_phdr / glibc backtrace   |

`-fno-optimize-sibling-calls` preserves call frames that tail-call
optimization would otherwise collapse, keeping stacks readable to the
above tools.

## Tests

Test programs live in `tests/`, with one program per data structure
(e.g., `tests/test_rb_tree.c`). A minimal header-only framework in
`tests/test.h` provides assertion macros (`TEST_ASSERT`,
`TEST_ASSERT_EQ`, `TEST_ASSERT_NULL`, `TEST_ASSERT_NOT_NULL`) and a
runner (`TEST_RUN`, `TEST_SUMMARY`).

Tests are an opt-in, separate build target. They are not part of the
shared/static library distribution and are not intended for drop-in
copy into other projects.

Each test program walks the data structure directly through an
independent in-test validator that does not rely on the library's own
validate routine. For `rb_tree`, the validator checks every RB
invariant (BST order, no two consecutive red nodes, equal black-height
across all paths, root and sentinel coloring) after every mutation.
Coverage spans the canonical insert and delete fixup cases (uncle-red
recolor, both zigzag directions, sibling rotations), sequential and
reverse-sequential patterns, alternating insert/delete on the same
key, growing-window alternations, stress sequences with several
hundred mixed operations, and pathological skewed-then-drain patterns.

Make:

```sh
make tests
./bin/test/test_rb_tree
```

CMake (gated by `BUILD_TESTS`):

```sh
cmake -B build -G Ninja -DBUILD_TESTS=ON
cmake --build build
./bin/test/test_rb_tree
```

The build system discovers `tests/*.c` automatically; each program
compiles to `bin/test/<name>` and links against the static library.

## Usage

`struct rb_node` carries a `uint64_t` key and the link / color fields
needed for tree traversal. Value storage is decided by the user, per
case: each tree's node size is fixed at compile time, but different
trees within the same build can have different sizes (slab-like).

The user wraps `struct rb_node` inside their own struct, picking the
inline value buffer size (or any other layout) that fits the case, and
recovers the wrapper from a returned `struct rb_node *` via
`container_of`. The library orders nodes by `node.key` directly; no
comparator callback is required.

Per-case node definitions via the convenience macro:

```c
#include "rb_tree.h"

RB_NODE_DEFINE(small,  32);
RB_NODE_DEFINE(medium, 256);
RB_NODE_DEFINE(large,  4096);

/* generates:
 *   struct rb_node_small  { struct rb_node node; char value[32];   };
 *   struct rb_node_medium { struct rb_node node; char value[256];  };
 *   struct rb_node_large  { struct rb_node node; char value[4096]; };
 */

struct rb_tree tree_s = { 0, };
struct rb_tree tree_m = { 0, };
rb_tree_init(&tree_s);
rb_tree_init(&tree_m);

struct rb_node_small *rs = malloc(sizeof(*rs));
rs->node.key = 42;
memcpy(rs->value, payload32, 32);
rb_tree_insert(&tree_s, &rs->node);

struct rb_node_medium *rm = malloc(sizeof(*rm));
rm->node.key = 42;
memcpy(rm->value, payload256, 256);
rb_tree_insert(&tree_m, &rm->node);

struct rb_node *hit_s = rb_tree_search(&tree_s, 42);
struct rb_node_small *found_s = container_of(hit_s, struct rb_node_small, node);
process(found_s->value);
```

Or define the wrapper struct directly when you want more than just a
value buffer (extra metadata, different field names, multiple
embedded `rb_node` members for membership in several trees, etc.):

```c
struct my_record {
    struct rb_node tree_link;
    char           payload[128];
    void          *external_handle;
    int            flags;
};

struct my_record *rec = malloc(sizeof(*rec));
rec->tree_link.key = 42;
memcpy(rec->payload, src, sizeof(rec->payload));
rec->external_handle = handle;
rb_tree_insert(&tree, &rec->tree_link);

struct rb_node *hit = rb_tree_search(&tree, 42);
struct my_record *found = container_of(hit, struct my_record, tree_link);
```

Key design points:

- Key is `uint64_t`, stored directly in `struct rb_node`; comparison
  is built in.
- Layout is predictable across x86_64, ARM64, and RISC-V LP64.
  `sizeof(struct rb_node)` is 64 bytes (one cache line on common
  configurations) with all field offsets fixed via explicit padding;
  `rb_color_t` is `uint8_t` (not an enum) so the storage size does not
  drift with compiler flags such as `-fshort-enums`.
- Value size is the user's choice per case, fixed at compile time.
  Different cases (different wrapper structs) can coexist in the same
  build — slab-like per-size pools.
- One allocation per element since `rb_node` is embedded in the user
  struct.
- Multiple trees per element are possible by embedding multiple
  `rb_node` members in the wrapper struct, one per tree.
- `container_of` is defined in `rb_tree.h` via standard `offsetof`;
  no compiler extensions required.

## Consumption Modes

### Shared / static library

Build, then link against `bin/libds.so` or `bin/libds.a` and add
`include/` to the compiler include path.

```sh
gcc -I/path/to/data-structure/include myprog.c \
    -L/path/to/data-structure/bin -lds -o myprog
```

### Drop-in source copy

Each data structure is self-contained as a header in `include/` and a
single translation unit in `src/`. Copy the matching pair into a target
project; no other files from this repository are required.

```
cp include/rb_tree.h /path/to/your/project/
cp src/rb_tree.c    /path/to/your/project/
```

## Adding Data Structures

Place the public header in `include/` and the implementation in
`src/`. The build system discovers sources via file globbing; no manual
registration is required. Place the corresponding test program in
`tests/`, which automatically picks up `tests/test.h`.