include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.8.1
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(Catch2)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)

add_executable(tests
    instructions.cpp
    e2e.cpp
    directives.cpp
)

target_link_libraries(tests PRIVATE ass_core ass_warnings Catch2::Catch2WithMain)

# Absolute path, so the tests do not depend on the working directory
target_compile_definitions(tests PRIVATE TEST_INPUTS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/inputs")

# Find test names when ctest runs, not after each build
catch_discover_tests(tests DISCOVERY_MODE PRE_TEST)
