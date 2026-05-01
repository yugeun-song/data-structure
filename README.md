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
make clean
```

### CMake

CMake auto-detects the compiler. Override via `CMAKE_C_COMPILER`:

```sh
cmake -B build -G Ninja                                                  # debug, autodetect
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release                       # release, autodetect
cmake -B build -G Ninja -DCMAKE_C_COMPILER=clang                         # debug, clang
cmake -B build -G Ninja -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

A preset for WSL environments is provided:

```sh
cmake --preset wsl-debug
cmake --build out/build/wsl-debug
```

### Compiler Flags

Common to both modes:

```
-Wall -Wextra -Wconversion -Wsign-conversion -fPIC
```

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
| gdb        | `-O0`, `-ggdb3`, frame pointers                                     |
| perf       | `-ggdb3`, frame pointers, `-fasynchronous-unwind-tables`            |
| gprof      | `-pg` (mcount instrumentation, generates gmon.out at exit)          |
| uftrace    | `-pg` (mcount-based runtime tracing)                                |
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