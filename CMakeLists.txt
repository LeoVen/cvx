cmake_minimum_required(VERSION 3.20)
project(cvx LANGUAGES C)
enable_testing()

find_package(Python3 REQUIRED COMPONENTS Interpreter)

add_subdirectory(tests)

