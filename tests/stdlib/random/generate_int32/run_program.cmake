if(NOT ZLANG_COMPILE OR NOT SOURCE OR NOT EXPECTED OR NOT WORK_DIR OR NOT C_COMPILER)
    message(FATAL_ERROR "run_program.cmake requires ZLANG_COMPILE, SOURCE, EXPECTED, WORK_DIR and C_COMPILER")
endif()
get_filename_component(_stem "${SOURCE}" NAME_WE)
set(_source "${WORK_DIR}/${_stem}.z")
set(_executable "${WORK_DIR}/${_stem}${EXECUTABLE_SUFFIX}")
file(MAKE_DIRECTORY "${WORK_DIR}")
configure_file("${SOURCE}" "${_source}" COPYONLY)
execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env "ZLANG_CC=${C_COMPILER}" "${ZLANG_COMPILE}" "${_source}"
    WORKING_DIRECTORY "${WORK_DIR}"
    RESULT_VARIABLE _result OUTPUT_VARIABLE _output ERROR_VARIABLE _error)
if(NOT _result EQUAL 0)
    message(FATAL_ERROR "zc failed: ${_error}${_output}")
endif()
execute_process(COMMAND "${_executable}" WORKING_DIRECTORY "${WORK_DIR}"
                RESULT_VARIABLE _run OUTPUT_VARIABLE _actual ERROR_VARIABLE _error)
if(NOT _run EQUAL 0)
    message(FATAL_ERROR "program failed: ${_error}")
endif()
file(READ "${EXPECTED}" _expected)
if(NOT _actual STREQUAL _expected)
    message(FATAL_ERROR "output mismatch\n--- expected ---\n${_expected}\n--- actual ---\n${_actual}")
endif()
