if(NOT ZLANG_BUILD_TESTS)
    return()
endif()

if(CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC" OR CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    set(_example_cc_style "MSVC")
else()
    set(_example_cc_style "GNU")
endif()

foreach(_category IN ITEMS algorithms features)
    file(GLOB _sources RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/${_category}"
         "${_category}/*/main.z")

    foreach(_source IN LISTS _sources)
        get_filename_component(_example_dir "${_source}" DIRECTORY)
        get_filename_component(_name "${_example_dir}" NAME)
        set(_test_name "zlang.examples.${_category}.${_name}")
        add_test(
            NAME "${_test_name}"
            COMMAND "${CMAKE_COMMAND}"
                    -DZLANG_COMPILE=$<TARGET_FILE:zc>
                    -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/${_category}/${_source}
                    -DGOLDEN_C=${CMAKE_CURRENT_SOURCE_DIR}/${_category}/${_example_dir}/main.c
                    -DEXPECTED=${CMAKE_CURRENT_SOURCE_DIR}/${_category}/${_example_dir}/expected.txt
                    -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/${_category}/${_example_dir}
                    -DC_COMPILER=${CMAKE_C_COMPILER}
                    -DCC_STYLE=${_example_cc_style}
                    -DEXECUTABLE_SUFFIX=${CMAKE_EXECUTABLE_SUFFIX}
                    -P ${CMAKE_CURRENT_SOURCE_DIR}/run_example.cmake)
        set_tests_properties("${_test_name}" PROPERTIES LABELS "module:examples;category:${_category}")
        if(ZLANG_SANITIZER_ENV)
            set_tests_properties("${_test_name}" PROPERTIES ENVIRONMENT "${ZLANG_SANITIZER_ENV}")
        endif()
    endforeach()
endforeach()
