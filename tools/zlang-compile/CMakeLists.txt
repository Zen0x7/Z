add_executable(zlang_compile main.c)
set_target_properties(zlang_compile PROPERTIES OUTPUT_NAME zlang-compile)
target_link_libraries(zlang_compile PRIVATE zlang::compiler_core)
target_compile_definitions(
    zlang_compile PRIVATE ZLANG_RUNTIME_INCLUDE_DIR="${CMAKE_SOURCE_DIR}/include")

install(TARGETS zlang_compile RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

if(ZLANG_BUILD_TESTS)
    configure_file(tests/emit_c.zlang ${CMAKE_CURRENT_BINARY_DIR}/emit_c.zlang COPYONLY)
    configure_file(tests/invalid_diagnostic.zlang
                   ${CMAKE_CURRENT_BINARY_DIR}/invalid_diagnostic.zlang COPYONLY)

    add_test(NAME zlang.cli.emit_c
             COMMAND $<TARGET_FILE:zlang_compile> ${CMAKE_CURRENT_BINARY_DIR}/emit_c.zlang --emit-c)
    add_test(NAME zlang.cli.emit_c.output
             COMMAND ${CMAKE_COMMAND}
                     -DEXPECTED_FILE=${CMAKE_CURRENT_BINARY_DIR}/emit_c.c
                     -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_emit_c.cmake)
    set_tests_properties(zlang.cli.emit_c PROPERTIES LABELS module:cli)
    set_tests_properties(zlang.cli.emit_c.output PROPERTIES LABELS module:cli
                         DEPENDS zlang.cli.emit_c)

    add_test(NAME zlang.cli.diagnostic_format
             COMMAND ${CMAKE_COMMAND}
                     -DZLANG_COMPILE=$<TARGET_FILE:zlang_compile>
                     -DSOURCE=${CMAKE_CURRENT_BINARY_DIR}/invalid_diagnostic.zlang
                     -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_diagnostic_format.cmake)
    set_tests_properties(zlang.cli.diagnostic_format PROPERTIES LABELS module:cli)

    if(ZLANG_SANITIZER_ENV)
        set_tests_properties(zlang.cli.emit_c zlang.cli.emit_c.output
                             zlang.cli.diagnostic_format
                             PROPERTIES ENVIRONMENT "${ZLANG_SANITIZER_ENV}")
    endif()
endif()
