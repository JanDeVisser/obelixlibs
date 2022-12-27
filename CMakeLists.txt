cmake_minimum_required(VERSION 3.16)
project(obelixlibs LANGUAGES CXX VERSION 1.0.0.0)

cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0115 NEW)
cmake_policy(SET CMP0135 NEW)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Wall")

set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR})

include_directories("${obelixlibs_SOURCE_DIR}/src" "${obelixlibs_SOURCE_DIR}")
add_subdirectory(lexer)
add_subdirectory(core)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_test(NAME CoreTest COMMAND CoreTest)
add_test(NAME LexerTest COMMAND LexerTest)
