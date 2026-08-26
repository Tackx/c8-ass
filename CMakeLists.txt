cmake_minimum_required(VERSION 3.28)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

project(c8-ass CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(ass)

target_sources(ass PRIVATE
    src/main.cpp
    src/ass.cpp
    src/instruction.cpp
    src/instruction.h
    src/label.h
    src/ass.h
)

target_include_directories(ass PRIVATE src)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(ass PRIVATE stdc++exp)
endif()