execute_process(
    COMMAND "${ZLANG_COMPILE}" "${SOURCE}" --emit-c
    RESULT_VARIABLE exit_code
    ERROR_VARIABLE error_output
    OUTPUT_QUIET)

if(NOT exit_code EQUAL 3)
    message(FATAL_ERROR "expected exit code 3, got '${exit_code}'")
endif()

if(NOT error_output MATCHES ":1:15: error\\[ZLANG1003\\]: invalid character")
    message(FATAL_ERROR "unexpected diagnostic output: '${error_output}'")
endif()
