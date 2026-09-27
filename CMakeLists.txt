cmake_minimum_required(VERSION 3.28)

project(c8-ass CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

# Creates the BUILD_TESTING option (default ON)
include(CTest)

# Warnings
add_library(ass_warnings INTERFACE)

if(MSVC)
    target_compile_options(ass_warnings INTERFACE /W4 /permissive-)
else()
    target_compile_options(ass_warnings INTERFACE
        -Wall -Wextra -Wconversion -Wsign-conversion -Wshadow -Wpedantic)
endif()

# Core library
add_library(ass_core STATIC
    src/ass.cpp
    src/instruction.cpp
    src/emitter.cpp
    src/parser.cpp
    src/util.cpp
    src/lexer.cpp
    src/loader.cpp
    src/flags.cpp
    src/logger.cpp
)

target_include_directories(ass_core PUBLIC src)
target_link_libraries(ass_core PRIVATE ass_warnings)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(ass_core PRIVATE stdc++exp)
endif()

# Executable
add_executable(ass src/main.cpp)
target_link_libraries(ass PRIVATE ass_core ass_warnings)

# Tests
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
