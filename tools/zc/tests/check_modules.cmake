if(NOT ZLANG_COMPILE OR NOT SOURCE OR NOT EXPECTED OR NOT WORK_DIR OR NOT C_COMPILER)
    message(FATAL_ERROR "check_modules.cmake requires ZLANG_COMPILE, SOURCE, EXPECTED, WORK_DIR and C_COMPILER")
endif()
get_filename_component(_stem "${SOURCE}" NAME_WE)
get_filename_component(_project_dir "${SOURCE}" DIRECTORY)
set(_source "${WORK_DIR}/${_stem}.z")
set(_executable "${WORK_DIR}/${_stem}${EXECUTABLE_SUFFIX}")
file(MAKE_DIRECTORY "${WORK_DIR}")
file(COPY "${_project_dir}/" DESTINATION "${WORK_DIR}")
execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env "ZLANG_CC=${C_COMPILER}" "${ZLANG_COMPILE}" "${_stem}.z"
    WORKING_DIRECTORY "${WORK_DIR}"
    RESULT_VARIABLE _compile OUTPUT_VARIABLE _compile_output ERROR_VARIABLE _error)
if(NOT _compile EQUAL 0)
    message(FATAL_ERROR "zc failed: ${_error}${_compile_output}")
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
file(READ "${WORK_DIR}/${_stem}.c" _generated)
foreach(_name
        app_local_m_lib_config_z_retries
        app_fn_m_lib_math_z_add_one
        app_fn_m_lib_math_z_zero
        app_local_m_lib_left_z_shared
        app_fn_m_lib_left_z_left_value
        app_local_m_lib_right_z_shared
        app_fn_m_lib_right_z_right_value
        app_struct_m_lib_types_z_point_t
        app_enum_m_lib_types_z_color_t)
    string(FIND "${_generated}" "${_name}" _position)
    if(_position LESS 0)
        message(FATAL_ERROR "generated C is missing canonical name ${_name}")
    endif()
endforeach()
