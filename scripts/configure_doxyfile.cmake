if(NOT DEFINED ZLANG_ROOT OR "${ZLANG_ROOT}" STREQUAL "")
    message(FATAL_ERROR "ZLANG_ROOT must be set")
endif()

include("${ZLANG_ROOT}/cmake/ZlangVersion.cmake")
set(ZLANG_GENERATED_INCLUDE_DIR "${ZLANG_ROOT}/build/generated/include")
configure_file("${ZLANG_ROOT}/include/zlang/version/version.h.in"
               "${ZLANG_GENERATED_INCLUDE_DIR}/zlang/version/version.h" @ONLY)
configure_file("${ZLANG_ROOT}/Doxyfile.in" "${ZLANG_ROOT}/build/Doxyfile" @ONLY)
