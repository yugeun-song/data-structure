cmake_minimum_required(VERSION 3.10)
project(DataStructure C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type (Debug or Release)" FORCE)
endif()

set(CMAKE_C_FLAGS_DEBUG "-O0 -ggdb3 -fno-omit-frame-pointer -fno-optimize-sibling-calls -fno-builtin -fasynchronous-unwind-tables -pg")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-pg -rdynamic")
set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "-pg")

add_compile_options(
    -Wall
    -Wextra
    -Wconversion
    -Wsign-conversion
)

include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB DS_SRCS "src/*.c")

add_library(ds_shared SHARED ${DS_SRCS})
set_target_properties(ds_shared PROPERTIES
    OUTPUT_NAME ds
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)

add_library(ds_static STATIC ${DS_SRCS})
set_target_properties(ds_static PROPERTIES
    OUTPUT_NAME ds
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)

option(BUILD_TESTS "Build test programs (not part of the library distribution)" OFF)

if(BUILD_TESTS)
    file(GLOB TEST_SRCS "tests/*.c")
    foreach(TEST_FILE ${TEST_SRCS})
        get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
        add_executable(${TEST_NAME} ${TEST_FILE})
        target_link_libraries(${TEST_NAME} PRIVATE ds_static)
        set_target_properties(${TEST_NAME} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/test"
        )
    endforeach()
endif()