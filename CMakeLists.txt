
cmake_minimum_required(VERSION 3.28)

if(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

project(c8-ass CXX)

# Enable testing for the project
include(CTest)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.8.1
)

FetchContent_MakeAvailable(Catch2)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)

add_library(ass_core STATIC
    src/ass.cpp
    src/instruction.cpp
    src/emitter.cpp
    src/parser.cpp
    src/util.cpp
    src/lexer.cpp
    src/loader.cpp
    src/flags.cpp
)
target_include_directories(ass_core PUBLIC src)

add_executable(ass src/main.cpp)
target_link_libraries(ass PRIVATE ass_core)

add_executable(tests tests/test.cpp)
target_link_libraries(tests PRIVATE ass_core Catch2::Catch2WithMain)

catch_discover_tests(tests)

if(MSVC)
    target_compile_options(ass PRIVATE /W4 /permissive-)
else()
    target_compile_options(ass PRIVATE
        -Wall -Wextra -Wconversion -Wsign-conversion -Wshadow -Wpedantic)
endif()


if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(ass PRIVATE stdc++exp)
endif()
