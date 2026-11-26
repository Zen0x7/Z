add_executable(zc main.c modules.c)
set_target_properties(zc PROPERTIES OUTPUT_NAME zc)
target_link_libraries(zc PRIVATE zlang::compiler_core)
target_compile_definitions(
    zc PRIVATE ZLANG_RUNTIME_INCLUDE_DIR="${CMAKE_SOURCE_DIR}/include")

install(TARGETS zc RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

if(ZLANG_BUILD_TESTS)
    configure_file(tests/emit_c.z ${CMAKE_CURRENT_BINARY_DIR}/emit_c.z COPYONLY)
    configure_file(tests/invalid_diagnostic.z
                   ${CMAKE_CURRENT_BINARY_DIR}/invalid_diagnostic.z COPYONLY)
    configure_file(tests/compiles.z ${CMAKE_CURRENT_BINARY_DIR}/compiles.z COPYONLY)
    configure_file(tests/comments.z ${CMAKE_CURRENT_BINARY_DIR}/comments.z COPYONLY)
    configure_file(tests/user_types.z ${CMAKE_CURRENT_BINARY_DIR}/user_types.z COPYONLY)
    configure_file(tests/multidimensional.z ${CMAKE_CURRENT_BINARY_DIR}/multidimensional.z COPYONLY)
    configure_file(tests/forward_calls.z ${CMAKE_CURRENT_BINARY_DIR}/forward_calls.z COPYONLY)
    configure_file(tests/tto.z ${CMAKE_CURRENT_BINARY_DIR}/tto.z COPYONLY)
    configure_file(tests/stateful_calls.z ${CMAKE_CURRENT_BINARY_DIR}/stateful_calls.z COPYONLY)
    configure_file(tests/program_options.z ${CMAKE_CURRENT_BINARY_DIR}/program_options.z COPYONLY)
    configure_file(tests/capabilities.z ${CMAKE_CURRENT_BINARY_DIR}/capabilities.z COPYONLY)
    configure_file(tests/jump.z ${CMAKE_CURRENT_BINARY_DIR}/jump.z COPYONLY)
    configure_file(tests/switch.z ${CMAKE_CURRENT_BINARY_DIR}/switch.z COPYONLY)

    add_test(NAME zlang.cli.emit_c
             COMMAND $<TARGET_FILE:zc> ${CMAKE_CURRENT_BINARY_DIR}/emit_c.z --emit-c)
    add_test(NAME zlang.cli.emit_c.output
             COMMAND ${CMAKE_COMMAND}
                     -DEXPECTED_FILE=${CMAKE_CURRENT_BINARY_DIR}/emit_c.c
                     -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_emit_c.cmake)
    set_tests_properties(zlang.cli.emit_c PROPERTIES LABELS module:cli)
    set_tests_properties(zlang.cli.emit_c.output PROPERTIES LABELS module:cli
                         DEPENDS zlang.cli.emit_c)

    add_test(NAME zlang.cli.diagnostic_format
             COMMAND ${CMAKE_COMMAND}
                     -DZLANG_COMPILE=$<TARGET_FILE:zc>
                     -DSOURCE=${CMAKE_CURRENT_BINARY_DIR}/invalid_diagnostic.z
                     -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_diagnostic_format.cmake)
    set_tests_properties(zlang.cli.diagnostic_format PROPERTIES LABELS module:cli)

    add_test(NAME zlang.cli.disable_comments
             COMMAND $<TARGET_FILE:zc> ${CMAKE_CURRENT_BINARY_DIR}/comments.z
                     --emit-c --disable-comments)
    add_test(NAME zlang.cli.disable_comments.output
             COMMAND ${CMAKE_COMMAND}
                     -DEMITTED_FILE=${CMAKE_CURRENT_BINARY_DIR}/comments.c
                     -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_disable_comments.cmake)
    set_tests_properties(zlang.cli.disable_comments PROPERTIES LABELS module:cli)
    set_tests_properties(zlang.cli.disable_comments.output PROPERTIES LABELS module:cli
                                                                  DEPENDS zlang.cli.disable_comments)

    add_test(NAME zlang.cli.compiles_generated_c
             COMMAND ${CMAKE_COMMAND}
                     -DZLANG_COMPILE=$<TARGET_FILE:zc>
                     -DSOURCE=${CMAKE_CURRENT_BINARY_DIR}/compiles.z
                     -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}
                     -DC_COMPILER=${CMAKE_C_COMPILER}
                     -DC_COMPILER_ID=${CMAKE_C_COMPILER_ID}
                     -DC_COMPILER_FRONTEND=${CMAKE_C_COMPILER_FRONTEND_VARIANT}
                     -DINCLUDE_DIR=${CMAKE_SOURCE_DIR}/include
                     -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_compile_generated_c.cmake)
    set_tests_properties(zlang.cli.compiles_generated_c PROPERTIES LABELS module:cli)

    add_test(NAME zlang.cli.compiles_switch
             COMMAND ${CMAKE_COMMAND}
                     -DZLANG_COMPILE=$<TARGET_FILE:zc>
                     -DSOURCE=${CMAKE_CURRENT_BINARY_DIR}/switch.z
                     -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}
                     -DC_COMPILER=${CMAKE_C_COMPILER}
                     -DC_COMPILER_ID=${CMAKE_C_COMPILER_ID}
                     -DC_COMPILER_FRONTEND=${CMAKE_C_COMPILER_FRONTEND_VARIANT}
                     -DINCLUDE_DIR=${CMAKE_SOURCE_DIR}/include
                     -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_compile_generated_c.cmake)
    set_tests_properties(zlang.cli.compiles_switch PROPERTIES LABELS module:cli)

    add_test(NAME zlang.cli.compiles_jump
             COMMAND ${CMAKE_COMMAND}
                     -DZLANG_COMPILE=$<TARGET_FILE:zc>
                     -DSOURCE=${CMAKE_CURRENT_BINARY_DIR}/jump.z
                     -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}
                     -DC_COMPILER=${CMAKE_C_COMPILER}
                     -DC_COMPILER_ID=${CMAKE_C_COMPILER_ID}
                     -DC_COMPILER_FRONTEND=${CMAKE_C_COMPILER_FRONTEND_VARIANT}
                     -DINCLUDE_DIR=${CMAKE_SOURCE_DIR}/include
                     -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_compile_generated_c.cmake)
    set_tests_properties(zlang.cli.compiles_jump PROPERTIES LABELS module:cli)

    add_test(NAME zlang.cli.compiles_capabilities
             COMMAND ${CMAKE_COMMAND}
                     -DZLANG_COMPILE=$<TARGET_FILE:zc>
                     -DSOURCE=${CMAKE_CURRENT_BINARY_DIR}/capabilities.z
                     -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}
                     -DC_COMPILER=${CMAKE_C_COMPILER}
                     -DC_COMPILER_ID=${CMAKE_C_COMPILER_ID}
                     -DC_COMPILER_FRONTEND=${CMAKE_C_COMPILER_FRONTEND_VARIANT}
                     -DINCLUDE_DIR=${CMAKE_SOURCE_DIR}/include
                     -DZLANG_EXTRA_FLAGS=--timing;--dump
                     -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_compile_generated_c.cmake)
    set_tests_properties(zlang.cli.compiles_capabilities PROPERTIES LABELS module:cli)

    add_test(NAME zlang.cli.compiles_user_types
             COMMAND ${CMAKE_COMMAND}
                     -DZLANG_COMPILE=$<TARGET_FILE:zc>
                     -DSOURCE=${CMAKE_CURRENT_BINARY_DIR}/user_types.z
                     -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}
                     -DC_COMPILER=${CMAKE_C_COMPILER}
                     -DC_COMPILER_ID=${CMAKE_C_COMPILER_ID}
                     -DC_COMPILER_FRONTEND=${CMAKE_C_COMPILER_FRONTEND_VARIANT}
                     -DINCLUDE_DIR=${CMAKE_SOURCE_DIR}/include
                     -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_compile_generated_c.cmake)
    set_tests_properties(zlang.cli.compiles_user_types PROPERTIES LABELS module:cli)

    add_test(NAME zlang.cli.compiles_multidimensional
             COMMAND ${CMAKE_COMMAND}
                     -DZLANG_COMPILE=$<TARGET_FILE:zc>
                     -DSOURCE=${CMAKE_CURRENT_BINARY_DIR}/multidimensional.z
                     -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}
                     -DC_COMPILER=${CMAKE_C_COMPILER}
                     -DC_COMPILER_ID=${CMAKE_C_COMPILER_ID}
                     -DC_COMPILER_FRONTEND=${CMAKE_C_COMPILER_FRONTEND_VARIANT}
                     -DINCLUDE_DIR=${CMAKE_SOURCE_DIR}/include
                     -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_compile_generated_c.cmake)
    set_tests_properties(zlang.cli.compiles_multidimensional PROPERTIES LABELS module:cli)

    add_test(NAME zlang.cli.compiles_forward_calls
             COMMAND ${CMAKE_COMMAND}
                     -DZLANG_COMPILE=$<TARGET_FILE:zc>
                     -DSOURCE=${CMAKE_CURRENT_BINARY_DIR}/forward_calls.z
                     -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}
                     -DC_COMPILER=${CMAKE_C_COMPILER}
                     -DC_COMPILER_ID=${CMAKE_C_COMPILER_ID}
                     -DC_COMPILER_FRONTEND=${CMAKE_C_COMPILER_FRONTEND_VARIANT}
                     -DINCLUDE_DIR=${CMAKE_SOURCE_DIR}/include
                     -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_compile_generated_c.cmake)
    set_tests_properties(zlang.cli.compiles_forward_calls PROPERTIES LABELS module:cli)

    add_test(NAME zlang.cli.tto
             COMMAND ${CMAKE_COMMAND}
                     -DZLANG_COMPILE=$<TARGET_FILE:zc>
                     -DSOURCE=${CMAKE_CURRENT_BINARY_DIR}/tto.z
                     -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}
                     -DC_COMPILER=${CMAKE_C_COMPILER}
                     -DC_COMPILER_ID=${CMAKE_C_COMPILER_ID}
                     -DC_COMPILER_FRONTEND=${CMAKE_C_COMPILER_FRONTEND_VARIANT}
                     -DINCLUDE_DIR=${CMAKE_SOURCE_DIR}/include
                     -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_tto.cmake)
    set_tests_properties(zlang.cli.tto PROPERTIES LABELS module:cli)

    add_test(NAME zlang.cli.compiles_stateful_calls
             COMMAND ${CMAKE_COMMAND}
                     -DZLANG_COMPILE=$<TARGET_FILE:zc>
                     -DSOURCE=${CMAKE_CURRENT_BINARY_DIR}/stateful_calls.z
                     -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}
                     -DC_COMPILER=${CMAKE_C_COMPILER}
                     -DC_COMPILER_ID=${CMAKE_C_COMPILER_ID}
                     -DC_COMPILER_FRONTEND=${CMAKE_C_COMPILER_FRONTEND_VARIANT}
                     -DINCLUDE_DIR=${CMAKE_SOURCE_DIR}/include
                     -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_compile_generated_c.cmake)
    set_tests_properties(zlang.cli.compiles_stateful_calls PROPERTIES LABELS module:cli)

    add_test(NAME zlang.cli.modules
             COMMAND ${CMAKE_COMMAND}
                     -DZLANG_COMPILE=$<TARGET_FILE:zc>
                     -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/tests/modules_project/main.z
                     -DEXPECTED=${CMAKE_CURRENT_SOURCE_DIR}/tests/modules_project/expected.txt
                     -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/modules_project
                     -DC_COMPILER=${CMAKE_C_COMPILER}
                     -DEXECUTABLE_SUFFIX=${CMAKE_EXECUTABLE_SUFFIX}
                     -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_modules.cmake)
    set_tests_properties(zlang.cli.modules PROPERTIES LABELS module:cli)

    foreach(_case modules_cycle modules_escape)
        string(REPLACE "_" "." _label "${_case}")
        if(_case STREQUAL modules_cycle)
            set(_message "cyclic module import")
        else()
            set(_message "cannot resolve module import")
        endif()
        add_test(NAME zlang.cli.${_case}
                 COMMAND ${CMAKE_COMMAND}
                         -DZLANG_COMPILE=$<TARGET_FILE:zc>
                         -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/tests/${_case}_project/main.z
                         -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/${_case}_project
                         -DC_COMPILER=${CMAKE_C_COMPILER}
                         -DMESSAGE=${_message}
                         -DEXECUTABLE_SUFFIX=${CMAKE_EXECUTABLE_SUFFIX}
                         -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_modules_failure.cmake)
        set_tests_properties(zlang.cli.${_case} PROPERTIES LABELS module:cli)
    endforeach()

    add_test(NAME zlang.cli.io_read
             COMMAND ${CMAKE_COMMAND}
                     -DZLANG_COMPILE=$<TARGET_FILE:zc>
                     -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/tests/io_read.z
                     -DINPUT=${CMAKE_CURRENT_SOURCE_DIR}/tests/io_read.input.txt
                     -DEXPECTED=${CMAKE_CURRENT_SOURCE_DIR}/tests/io_read.expected.txt
                     -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/io_read
                     -DC_COMPILER=${CMAKE_C_COMPILER}
                     -DEXECUTABLE_SUFFIX=${CMAKE_EXECUTABLE_SUFFIX}
                     -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_io_read.cmake)
    set_tests_properties(zlang.cli.io_read PROPERTIES LABELS module:cli)

    add_test(NAME zlang.cli.legacy_std_import
             COMMAND ${CMAKE_COMMAND}
                     -DZLANG_COMPILE=$<TARGET_FILE:zc>
                     -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/tests/legacy_std_import.z
                     -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/legacy_std_import
                     -DC_COMPILER=${CMAKE_C_COMPILER}
                     -DMESSAGE=cannot resolve module import
                     -DEXECUTABLE_SUFFIX=${CMAKE_EXECUTABLE_SUFFIX}
                     -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_modules_failure.cmake)
    set_tests_properties(zlang.cli.legacy_std_import PROPERTIES LABELS module:cli)

    add_test(NAME zlang.cli.program_options
             COMMAND ${CMAKE_COMMAND}
                     -DZLANG_COMPILE=$<TARGET_FILE:zc>
                     -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/tests/program_options.z
                     -DEXPECTED=${CMAKE_CURRENT_SOURCE_DIR}/tests/program_options.expected.txt
                     -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/program_options
                     -DC_COMPILER=${CMAKE_C_COMPILER}
                     -DINCLUDE_DIR=${CMAKE_SOURCE_DIR}/include
                     -DEXECUTABLE_SUFFIX=${CMAKE_EXECUTABLE_SUFFIX}
                     -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_program_options.cmake)
    set_tests_properties(zlang.cli.program_options PROPERTIES LABELS module:cli)

    if(ZLANG_SANITIZER_ENV)
        set_tests_properties(zlang.cli.emit_c zlang.cli.emit_c.output zlang.cli.diagnostic_format
                             zlang.cli.disable_comments zlang.cli.disable_comments.output
                             zlang.cli.compiles_generated_c zlang.cli.compiles_user_types
                             zlang.cli.compiles_multidimensional
                             zlang.cli.compiles_forward_calls
                             zlang.cli.tto
                             zlang.cli.compiles_stateful_calls
                             zlang.cli.modules
                             zlang.cli.modules_cycle
                             zlang.cli.modules_escape
                             zlang.cli.legacy_std_import
                             zlang.cli.io_read
                             zlang.cli.program_options
                             PROPERTIES ENVIRONMENT "${ZLANG_SANITIZER_ENV}")
    endif()
endif()
