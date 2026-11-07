cmake_minimum_required(VERSION 3.20)
project(cvx LANGUAGES C)
enable_testing()

find_package(Python3 REQUIRED COMPONENTS Interpreter)

option(CVX_SANITIZE "Build with ASan and UBSan" ON)
if(CVX_SANITIZE)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -Wno-c23-extensions)
    add_link_options(-fsanitize=address,undefined)
endif()

add_subdirectory(tests)

