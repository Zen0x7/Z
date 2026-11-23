# Registers one test module: a dedicated executable plus one CTest test per
# case. Every module follows the same layout, so adding a module never touches
# the files of another one.
#
# Usage (from tests/<module>/CMakeLists.txt):
#   zen_add_test_module(<module>)
#
# Expected files in the current source directory:
#   <module>_tests.c   test functions and main()
#   test_cases.h       ZEN_TEST_CASE(name) lines (single source of truth)
function(zen_add_test_module module_name)
    set(_target zen_${module_name}_tests)
    add_executable(${_target} ${module_name}_tests.c)
    target_link_libraries(${_target} PRIVATE zen::zen)
    if(ZEN_SANITIZER STREQUAL "shadow-call-stack")
        # Rename the C entry point so the assembly wrapper can set x18 in the
        # same thread, immediately before any instrumented code runs.
        target_compile_definitions(${_target} PRIVATE main=zen_main_impl ZEN_SCS_MAIN=1)
        target_link_libraries(${_target} PRIVATE zen_scs_runtime)
    endif()

    file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/test_cases.h" _case_lines
         REGEX "^ZEN_TEST_CASE\\([a-z0-9_]+\\)$")

    set(_module_tests)
    foreach(_case_line IN LISTS _case_lines)
        string(REGEX REPLACE "^ZEN_TEST_CASE\\(([a-z0-9_]+)\\)$" "\\1"
               _case "${_case_line}")
        set(_test_name "zen.${module_name}.${_case}")
        add_test(NAME "${_test_name}" COMMAND ${_target} --case "${_case}")
        set_tests_properties("${_test_name}" PROPERTIES LABELS "module:${module_name}")
        list(APPEND _module_tests "${_test_name}")
    endforeach()

    if(ZEN_SANITIZER_ENV)
        set_tests_properties(${_module_tests} PROPERTIES ENVIRONMENT "${ZEN_SANITIZER_ENV}")
    endif()

    set_property(GLOBAL APPEND PROPERTY ZEN_TEST_TARGETS ${_target})
endfunction()
