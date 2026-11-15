cmake_minimum_required(VERSION 3.21)
project(cvx LANGUAGES C)
enable_testing()

find_package(Python3 REQUIRED COMPONENTS Interpreter)

option(CVX_SANITIZE "Build with ASan and UBSan" ON)
if(CVX_SANITIZE)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

option(CVX_COVERAGE "Build with coverage instrumentation" OFF)
if(CVX_COVERAGE)
    add_compile_options(--coverage -O0 -g)
    add_link_options(--coverage)
endif()

add_subdirectory(tests)

