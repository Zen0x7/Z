cmake_minimum_required(VERSION 3.21)

project(zen VERSION 0.0.0 LANGUAGES C)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Sanitizer flags must be known before warnings/coverage are added.
include(ZenSanitizers)
include(ZenWarnings)
include(ZenCoverage)

add_compile_options(${ZEN_SANITIZER_FLAGS})
add_link_options(${ZEN_SANITIZER_FLAGS} ${ZEN_SANITIZER_LINK_FLAGS})

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_library(zen INTERFACE)
add_library(zen::zen ALIAS zen)

target_include_directories(zen INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

# C99 is the canonical standard: static inline + <stdint.h> are enough and it is
# the widest common denominator across every platform in the CI matrix.
target_compile_features(zen INTERFACE c_std_99)

install(TARGETS zen EXPORT zenTargets)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(EXPORT zenTargets
        FILE zenTargets.cmake
        NAMESPACE zen::
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/zen")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/zenConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/zenConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/zen")
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/zenConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMajorVersion)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/zenConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/zenConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/zen")

option(ZEN_BUILD_TESTS "Build the zen test suite" ON)

if(ZEN_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

option(ZEN_BUILD_FUZZERS "Build libFuzzer targets (Clang only)" OFF)

if(ZEN_BUILD_FUZZERS)
    if(NOT CMAKE_C_COMPILER_ID MATCHES "Clang")
        message(FATAL_ERROR "ZEN_BUILD_FUZZERS requires Clang")
    endif()
    add_subdirectory(fuzz)
endif()

if(ZEN_COVERAGE)
    set(_zen_cov_args
        --root "${CMAKE_SOURCE_DIR}"
        --gcov-object-directory "${CMAKE_BINARY_DIR}"
        --filter "${CMAKE_SOURCE_DIR}/include/.*"
        --exclude "${CMAKE_SOURCE_DIR}/tests/.*"
        --gcov-ignore-errors=no_working_dir_found)

    # The build tree is passed as gcovr's positional search path so stale
    # build directories in the source tree are never scanned.
    get_property(_zen_test_targets GLOBAL PROPERTY ZEN_TEST_TARGETS)

    add_custom_target(zen_coverage
        DEPENDS ${_zen_test_targets}
        COMMAND "${CMAKE_CTEST_COMMAND}" --output-on-failure
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_BINARY_DIR}/coverage"
        COMMAND "${GCOVR_EXECUTABLE}" ${_zen_cov_args} --txt --fail-under-line=80
                "${CMAKE_BINARY_DIR}"
        COMMAND "${GCOVR_EXECUTABLE}" ${_zen_cov_args}
                --xml-pretty --output "${CMAKE_BINARY_DIR}/coverage/coverage.xml"
                "${CMAKE_BINARY_DIR}"
        COMMAND "${GCOVR_EXECUTABLE}" ${_zen_cov_args}
                --html-details "${CMAKE_BINARY_DIR}/coverage/index.html"
                "${CMAKE_BINARY_DIR}"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        COMMENT "Running tests and generating gcovr coverage reports"
        VERBATIM)
endif()
