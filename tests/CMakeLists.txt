set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    ${PROJECT_SOURCE_DIR}/tests/config.json
)

execute_process(
    COMMAND ${Python3_EXECUTABLE} cvx_generator.py
        --config tests/config.json
        --compiler gcc
        --list-outputs
        --output ${CMAKE_CURRENT_BINARY_DIR}/generated
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE outs
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(REPLACE "\n" ";" outs "${outs}")
foreach(out IN LISTS outs)
    message(STATUS "${out}")
endforeach()

set(sources "${outs}")
list(FILTER sources INCLUDE REGEX "\\.c$")

file(GLOB templates CONFIGURE_DEPENDS
    ${PROJECT_SOURCE_DIR}/cvx/templates/*.h
    ${PROJECT_SOURCE_DIR}/cvx/templates/*.c
)
add_custom_command(
    OUTPUT ${outs}
    COMMAND ${Python3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/cvx_generator.py
        --config ${PROJECT_SOURCE_DIR}/tests/config.json
        --compiler gcc
        --output ${CMAKE_CURRENT_BINARY_DIR}/generated
    DEPENDS ${templates}
        ${PROJECT_SOURCE_DIR}/cvx_generator.py
        ${PROJECT_SOURCE_DIR}/cvx/names.h
        ${PROJECT_SOURCE_DIR}/tests/config.json
)

add_library(cvx_generated STATIC ${sources})
target_include_directories(cvx_generated PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}/generated/includes
    ${PROJECT_SOURCE_DIR}
)

foreach(ds IN ITEMS hamt)
    add_executable(${ds}_tests src/${ds}_tests.c)
    target_link_libraries(${ds}_tests PRIVATE cvx_generated)
    target_include_directories(${ds}_tests PRIVATE include)
    add_test(NAME ${ds} COMMAND ${ds}_tests)
endforeach()
