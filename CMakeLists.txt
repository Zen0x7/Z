cmake_minimum_required(VERSION 3.21)

include(cmake/ZlangVersion.cmake)
project(zlang VERSION ${ZLANG_PROJECT_VERSION} LANGUAGES C)

set(ZLANG_GENERATED_INCLUDE_DIR "${CMAKE_BINARY_DIR}/generated/include")
configure_file(Doxyfile.in ${CMAKE_BINARY_DIR}/Doxyfile @ONLY)
configure_file(include/zlang/version/version.h.in
               ${CMAKE_BINARY_DIR}/generated/include/zlang/version/version.h @ONLY)
configure_file(include/zlang/version/version_config.h.in
               ${CMAKE_BINARY_DIR}/generated/include/zlang/version/version_config.h @ONLY)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Sanitizer flags must be known before warnings/coverage are added.
include(ZlangSanitizers)
include(ZlangWarnings)
include(ZlangCoverage)

add_compile_options(${ZLANG_SANITIZER_FLAGS})
add_link_options(${ZLANG_SANITIZER_FLAGS} ${ZLANG_SANITIZER_LINK_FLAGS})

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_library(zlang INTERFACE)
add_library(zlang::zlang ALIAS zlang)

target_include_directories(zlang INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated/include>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

# C99 is the canonical standard: static inline + <stdint.h> are enough and it is
# the widest common denominator across every platform in the CI matrix.
target_compile_features(zlang INTERFACE c_std_99)

install(TARGETS zlang EXPORT zlangTargets)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
        PATTERN "*.in" EXCLUDE)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/generated/include/zlang/version/version.h"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/zlang/version")
install(EXPORT zlangTargets
        FILE zlangTargets.cmake
        NAMESPACE zlang::
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/zlang")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/zlangConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/zlangConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/zlang")
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/zlangConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMajorVersion)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/zlangConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/zlangConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/zlang")

option(ZLANG_BUILD_TESTS "Build the zlang test suite" ON)

if(ZLANG_BUILD_TESTS)
    enable_testing()
endif()

add_subdirectory(compiler)
add_subdirectory(tools/zlang-compile)
add_subdirectory(examples)

if(ZLANG_BUILD_TESTS)
    add_subdirectory(tests)
endif()

option(ZLANG_BUILD_FUZZERS "Build libFuzzer targets (Clang only)" OFF)

if(ZLANG_BUILD_FUZZERS)
    if(NOT CMAKE_C_COMPILER_ID MATCHES "Clang")
        message(FATAL_ERROR "ZLANG_BUILD_FUZZERS requires Clang")
    endif()
    add_subdirectory(fuzz)
endif()

if(ZLANG_COVERAGE)
    set(_zlang_cov_args
        --root "${CMAKE_SOURCE_DIR}"
        --gcov-object-directory "${CMAKE_BINARY_DIR}"
        --filter "${CMAKE_SOURCE_DIR}/include/.*"
        --exclude "${CMAKE_SOURCE_DIR}/tests/.*"
        --gcov-ignore-errors=no_working_dir_found)

    # The build tree is passed as gcovr's positional search path so stale
    # build directories in the source tree are never scanned.
    get_property(_zlang_test_targets GLOBAL PROPERTY ZLANG_TEST_TARGETS)
    list(APPEND _zlang_test_targets zlang_compile)

    add_custom_target(zlang_coverage
        DEPENDS ${_zlang_test_targets}
        COMMAND "${CMAKE_CTEST_COMMAND}" --output-on-failure
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_BINARY_DIR}/coverage"
        COMMAND "${GCOVR_EXECUTABLE}" ${_zlang_cov_args} --txt --fail-under-line=80
                "${CMAKE_BINARY_DIR}"
        COMMAND "${GCOVR_EXECUTABLE}" ${_zlang_cov_args}
                --xml-pretty --output "${CMAKE_BINARY_DIR}/coverage/coverage.xml"
                "${CMAKE_BINARY_DIR}"
        COMMAND "${GCOVR_EXECUTABLE}" ${_zlang_cov_args}
                --html-details "${CMAKE_BINARY_DIR}/coverage/index.html"
                "${CMAKE_BINARY_DIR}"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        COMMENT "Running tests and generating gcovr coverage reports"
        VERBATIM)
endif()
