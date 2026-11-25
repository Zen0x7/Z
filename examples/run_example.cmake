if(NOT ZLANG_COMPILE OR NOT SOURCE OR NOT GOLDEN_C OR NOT EXPECTED OR NOT WORK_DIR OR NOT C_COMPILER)
    message(FATAL_ERROR "run_example.cmake requires ZLANG_COMPILE, SOURCE, GOLDEN_C, EXPECTED, WORK_DIR and C_COMPILER")
endif()

get_filename_component(_stem "${SOURCE}" NAME_WE)
set(_source "${WORK_DIR}/${_stem}.zlang")
set(_generated_c "${WORK_DIR}/${_stem}.c")
set(_executable "${WORK_DIR}/${_stem}${EXECUTABLE_SUFFIX}")

file(MAKE_DIRECTORY "${WORK_DIR}")
configure_file("${SOURCE}" "${_source}" COPYONLY)

execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env "ZLANG_CC=${C_COMPILER}" "${ZLANG_COMPILE}" "${_source}"
    WORKING_DIRECTORY "${WORK_DIR}"
    RESULT_VARIABLE _compile_result
    OUTPUT_VARIABLE _compile_output
    ERROR_VARIABLE _compile_error)
if(NOT _compile_result EQUAL 0)
    message(FATAL_ERROR "zlang-compile failed: ${_compile_error}${_compile_output}")
endif()
if(NOT EXISTS "${_generated_c}")
    message(FATAL_ERROR "generated C not found at ${_generated_c}")
endif()
if(NOT EXISTS "${_executable}")
    message(FATAL_ERROR "example executable not found at ${_executable}")
endif()

file(READ "${_generated_c}" _actual_c)
file(READ "${GOLDEN_C}" _expected_c)
string(REPLACE "\r\n" "\n" _actual_c "${_actual_c}")
string(REPLACE "\r\n" "\n" _expected_c "${_expected_c}")
if(NOT _actual_c STREQUAL _expected_c)
    message(FATAL_ERROR "generated C mismatch for ${_stem}")
endif()

execute_process(
    COMMAND "${_executable}"
    WORKING_DIRECTORY "${WORK_DIR}"
    RESULT_VARIABLE _run_result
    OUTPUT_VARIABLE _actual
    ERROR_VARIABLE _run_error)
if(NOT _run_result EQUAL 0)
    message(FATAL_ERROR "example failed with status ${_run_result}: ${_run_error}")
endif()

file(READ "${EXPECTED}" _expected)
string(REPLACE "\r\n" "\n" _actual "${_actual}")
string(REPLACE "\r\n" "\n" _expected "${_expected}")
if(NOT _actual STREQUAL _expected)
    message(FATAL_ERROR "output mismatch\n--- expected ---\n${_expected}\n--- actual ---\n${_actual}")
endif()
