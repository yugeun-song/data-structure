cmake_minimum_required(VERSION 3.10)
project(DataStructure C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_compile_options(
    -Wall
    -Wextra
    -Wconversion
    -Wsign-conversion
    -O0
    -ggdb3
    -fno-omit-frame-pointer
    -fno-optimize-sibling-calls
    -fasynchronous-unwind-tables
)

include_directories(${CMAKE_SOURCE_DIR}/include)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

file(GLOB DS_SRCS "src/*.c")

add_library(ds_shared SHARED ${DS_SRCS})
set_target_properties(ds_shared PROPERTIES
    OUTPUT_NAME ds
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)
target_link_libraries(ds_shared PUBLIC Threads::Threads)

add_library(ds_static STATIC ${DS_SRCS})
set_target_properties(ds_static PROPERTIES
    OUTPUT_NAME ds
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)
target_link_libraries(ds_static PUBLIC Threads::Threads)

file(GLOB TEST_SRCS "tests/*.c")
foreach(TEST_FILE ${TEST_SRCS})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_FILE})
    target_link_libraries(${TEST_NAME} PRIVATE ds_static Threads::Threads)
    set_target_properties(${TEST_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/test"
    )
endforeach()