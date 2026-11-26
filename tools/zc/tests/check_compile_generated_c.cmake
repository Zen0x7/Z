if(NOT EXISTS "${SOURCE}")
    message(FATAL_ERROR "Missing Zlang source at ${SOURCE}")
endif()

get_filename_component(_stem "${SOURCE}" NAME_WE)
set(_emitted "${WORK_DIR}/${_stem}.c")
set(_object "${WORK_DIR}/${_stem}.obj")

set(_extra_flags)
if(ZLANG_EXTRA_FLAGS)
    set(_extra_flags ${ZLANG_EXTRA_FLAGS})
endif()

execute_process(
    COMMAND "${ZLANG_COMPILE}" "${SOURCE}" --emit-c ${_extra_flags}
    WORKING_DIRECTORY "${WORK_DIR}"
    RESULT_VARIABLE _emit_result
    OUTPUT_VARIABLE _emit_output
    ERROR_VARIABLE _emit_error)
if(NOT _emit_result EQUAL 0)
    message(FATAL_ERROR "zc failed: ${_emit_error}${_emit_output}")
endif()
if(NOT EXISTS "${_emitted}")
    message(FATAL_ERROR "zc did not write ${_emitted}")
endif()

if(C_COMPILER_ID MATCHES "MSVC" OR C_COMPILER_FRONTEND STREQUAL "MSVC")
    execute_process(
        COMMAND "${C_COMPILER}" /nologo /W4 /WX /c "/I${INCLUDE_DIR}"
                "/Fo${_object}" "${_emitted}"
        RESULT_VARIABLE _compile_result
        OUTPUT_VARIABLE _compile_output
        ERROR_VARIABLE _compile_error)
else()
    execute_process(
        COMMAND "${C_COMPILER}" -std=c99 -Wall -Wextra -Werror -pedantic
                "-I${INCLUDE_DIR}" -c "${_emitted}" -o "${_object}"
        RESULT_VARIABLE _compile_result
        OUTPUT_VARIABLE _compile_output
        ERROR_VARIABLE _compile_error)
endif()

if(NOT _compile_result EQUAL 0)
    message(FATAL_ERROR "Generated C failed to compile: ${_compile_error}${_compile_output}")
endif()
