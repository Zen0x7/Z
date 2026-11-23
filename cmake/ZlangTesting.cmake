# Registers one test module: a dedicated executable plus one CTest test per
# case. Every module follows the same layout, so adding a module never touches
# the files of another one.
#
# Usage (from tests/<module>/CMakeLists.txt):
#   zlang_add_test_module(<module>)
#
# Expected files in the current source directory:
#   <module>_tests.c   test functions and main()
#   test_cases.h       ZLANG_TEST_CASE(name) lines (single source of truth)
function(zlang_add_test_module module_name)
    set(_target zlang_${module_name}_tests)
    add_executable(${_target} ${module_name}_tests.c)
    target_link_libraries(${_target} PRIVATE zlang::zlang)

    file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/test_cases.h" _case_lines
         REGEX "^ZLANG_TEST_CASE\\([a-z0-9_]+\\)$")

    set(_module_tests)
    foreach(_case_line IN LISTS _case_lines)
        string(REGEX REPLACE "^ZLANG_TEST_CASE\\(([a-z0-9_]+)\\)$" "\\1"
               _case "${_case_line}")
        set(_test_name "zlang.${module_name}.${_case}")
        add_test(NAME "${_test_name}" COMMAND ${_target} --case "${_case}")
        set_tests_properties("${_test_name}" PROPERTIES LABELS "module:${module_name}")
        list(APPEND _module_tests "${_test_name}")
    endforeach()

    if(ZLANG_SANITIZER_ENV)
        set_tests_properties(${_module_tests} PROPERTIES ENVIRONMENT "${ZLANG_SANITIZER_ENV}")
    endif()

    set_property(GLOBAL APPEND PROPERTY ZLANG_TEST_TARGETS ${_target})
endfunction()
