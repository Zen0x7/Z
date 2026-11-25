if(NOT ZLANG_BUILD_TESTS)
    return()
endif()

foreach(_category IN ITEMS algorithms features)
    file(GLOB _sources RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/${_category}" "${_category}/*.zlang")

    foreach(_example IN LISTS _sources)
        get_filename_component(_name "${_example}" NAME_WE)
        set(_test_name "zlang.examples.${_category}.${_name}")
        add_test(
            NAME "${_test_name}"
            COMMAND "${CMAKE_COMMAND}"
                    -DZLANG_COMPILE=$<TARGET_FILE:zlang_compile>
                    -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/${_category}/${_example}
                    -DGOLDEN_C=${CMAKE_CURRENT_SOURCE_DIR}/${_category}/${_name}.c
                    -DEXPECTED=${CMAKE_CURRENT_SOURCE_DIR}/${_category}/${_name}.expected.txt
                    -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/${_category}/${_name}
                    -DC_COMPILER=${CMAKE_C_COMPILER}
                    -DEXECUTABLE_SUFFIX=${CMAKE_EXECUTABLE_SUFFIX}
                    -P ${CMAKE_CURRENT_SOURCE_DIR}/run_example.cmake)
        set_tests_properties("${_test_name}" PROPERTIES LABELS "module:examples;category:${_category}")
        if(ZLANG_SANITIZER_ENV)
            set_tests_properties("${_test_name}" PROPERTIES ENVIRONMENT "${ZLANG_SANITIZER_ENV}")
        endif()
    endforeach()
endforeach()
