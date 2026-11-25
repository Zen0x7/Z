if(NOT ZLANG_COMPILE OR NOT SOURCE OR NOT EXPECTED OR NOT WORK_DIR OR NOT C_COMPILER)
    message(FATAL_ERROR "check_program_options.cmake requires ZLANG_COMPILE, SOURCE, EXPECTED, WORK_DIR and C_COMPILER")
endif()
get_filename_component(_stem "${SOURCE}" NAME_WE)
set(_executable "${WORK_DIR}/${_stem}${EXECUTABLE_SUFFIX}")
file(MAKE_DIRECTORY "${WORK_DIR}")
configure_file("${SOURCE}" "${WORK_DIR}/${_stem}.z" COPYONLY)
execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env "ZLANG_CC=${C_COMPILER}" "${ZLANG_COMPILE}" "${_stem}.z" --emit-c
    WORKING_DIRECTORY "${WORK_DIR}"
    RESULT_VARIABLE _compile OUTPUT_VARIABLE _compile_output ERROR_VARIABLE _error)
if(NOT _compile EQUAL 0)
    message(FATAL_ERROR "zc failed: ${_error}${_compile_output}")
endif()
execute_process(
    COMMAND "${C_COMPILER}" -std=c99 -O2 -I "${INCLUDE_DIR}" "${WORK_DIR}/${_stem}.c" -o "${_executable}"
    RESULT_VARIABLE _build OUTPUT_VARIABLE _build_output ERROR_VARIABLE _build_error)
if(NOT _build EQUAL 0)
    message(FATAL_ERROR "C compiler failed: ${_build_error}${_build_output}")
endif()
execute_process(
    COMMAND "${_executable}" alpha --name beta --verbose
    RESULT_VARIABLE _run OUTPUT_VARIABLE _actual ERROR_VARIABLE _error)
if(NOT _run EQUAL 0)
    message(FATAL_ERROR "program failed: ${_error}")
endif()
file(READ "${EXPECTED}" _expected)
string(REPLACE "\r\n" "\n" _actual "${_actual}")
string(REPLACE "\r\n" "\n" _expected "${_expected}")
if(NOT _actual STREQUAL _expected)
    message(FATAL_ERROR "output mismatch\n--- expected ---\n${_expected}\n--- actual ---\n${_actual}")
endif()
