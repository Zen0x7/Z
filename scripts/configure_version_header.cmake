if(NOT DEFINED ZLANG_ROOT OR "${ZLANG_ROOT}" STREQUAL "")
    message(FATAL_ERROR "ZLANG_ROOT must be set")
endif()
if(NOT DEFINED ZLANG_OUTPUT OR "${ZLANG_OUTPUT}" STREQUAL "")
    message(FATAL_ERROR "ZLANG_OUTPUT must be set")
endif()

include("${ZLANG_ROOT}/cmake/ZlangVersion.cmake")
configure_file("${ZLANG_ROOT}/include/zlang/version/version.h.in"
               "${ZLANG_OUTPUT}/zlang/version/version.h" @ONLY)
