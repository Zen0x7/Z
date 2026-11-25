zlang_add_test_module(zlang_stdlib_random_generate_int32)

add_test(
    NAME zlang.stdlib.random.generate_int32.basic
    COMMAND ${CMAKE_COMMAND}
            -DZLANG_COMPILE=$<TARGET_FILE:zlang_compile>
            -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/programs/basic/main.zlang
            -DEXPECTED=${CMAKE_CURRENT_SOURCE_DIR}/programs/basic/expected.txt
            -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/basic
            -DC_COMPILER=${CMAKE_C_COMPILER}
            -DEXECUTABLE_SUFFIX=${CMAKE_EXECUTABLE_SUFFIX}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/run_program.cmake)
set_tests_properties(zlang.stdlib.random.generate_int32.basic PROPERTIES
                     LABELS "module:stdlib;domain:random;function:generate_int32")
