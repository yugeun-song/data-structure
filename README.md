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
├── tests                     Standalone test programs
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

- GCC
- GNU Make or CMake (>= 3.10)
- pthreads

### Make

```sh
make           # builds shared and static libraries
make tests     # builds test programs
make clean
```

### CMake

```sh
cmake -B build -G Ninja
cmake --build build
```

A preset for WSL environments is provided:

```sh
cmake --preset wsl-debug
cmake --build out/build/wsl-debug
```

### Compiler Flags

```
-Wall -Wextra -Wconversion -Wsign-conversion
-O0 -ggdb3
-fno-omit-frame-pointer -fno-optimize-sibling-calls -fasynchronous-unwind-tables
-fPIC
```

Optimization is disabled. Debug symbols and frame pointers are
preserved for accurate stack traces under GDB, Valgrind, and perf.

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
`tests/`.