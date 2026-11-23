# Single-entry sanitizer configuration. Combining sanitizers is deliberately not
# supported (the matrix runs each one separately); in particular address and
# undefined are not combined because they are already independent jobs.
include(CheckCSourceCompiles)

set(ZEN_SANITIZER "none" CACHE STRING
    "Sanitizer: none, address, undefined, thread, leak, integer, cfi, hwaddress, shadow-call-stack")
set_property(CACHE ZEN_SANITIZER PROPERTY STRINGS
    none address undefined thread leak integer cfi hwaddress shadow-call-stack)

set(ZEN_SANITIZER_FLAGS "")
set(ZEN_SANITIZER_LINK_FLAGS "")
set(ZEN_SANITIZER_ENV "")

set(_zen_is_clang FALSE)
set(_zen_is_gcc FALSE)
set(_zen_is_msvc FALSE)
if(CMAKE_C_COMPILER_ID MATCHES "Clang")
    set(_zen_is_clang TRUE)
elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(_zen_is_gcc TRUE)
elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    set(_zen_is_msvc TRUE)
endif()

set(_zen_is_aarch64 FALSE)
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64|ARM64)$")
    set(_zen_is_aarch64 TRUE)
endif()

if(ZEN_SANITIZER STREQUAL "none")
    # Nothing to do.

elseif(ZEN_SANITIZER STREQUAL "address")
    if(_zen_is_msvc)
        # /WX promotes C5072 to an error, so MSVC ASAN always needs debug info
        # even in Release builds.
        list(APPEND ZEN_SANITIZER_FLAGS /fsanitize=address /Zi)
        list(APPEND ZEN_SANITIZER_LINK_FLAGS /DEBUG)
    elseif(_zen_is_gcc OR _zen_is_clang)
        list(APPEND ZEN_SANITIZER_FLAGS -fsanitize=address -fno-omit-frame-pointer)
    else()
        message(FATAL_ERROR "address sanitizer requires GCC, Clang or MSVC")
    endif()
    # Neither MSVC's runtime nor Apple's Clang runtime supports detect_leaks.
    if(_zen_is_msvc OR APPLE)
        list(APPEND ZEN_SANITIZER_ENV "ASAN_OPTIONS=halt_on_error=1:abort_on_error=1")
    else()
        list(APPEND ZEN_SANITIZER_ENV "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1:abort_on_error=1")
        list(APPEND ZEN_SANITIZER_ENV "LSAN_OPTIONS=halt_on_error=1")
    endif()

elseif(ZEN_SANITIZER STREQUAL "undefined")
    if(NOT (_zen_is_gcc OR _zen_is_clang))
        message(FATAL_ERROR "undefined sanitizer requires GCC or Clang")
    endif()
    list(APPEND ZEN_SANITIZER_FLAGS
        -fsanitize=undefined -fno-sanitize-recover=all -fno-omit-frame-pointer)
    list(APPEND ZEN_SANITIZER_ENV "UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1")

elseif(ZEN_SANITIZER STREQUAL "thread")
    if(NOT (_zen_is_gcc OR _zen_is_clang))
        message(FATAL_ERROR "thread sanitizer requires GCC or Clang")
    endif()
    if(APPLE)
        message(FATAL_ERROR "thread sanitizer is not supported by Clang on macOS")
    endif()
    list(APPEND ZEN_SANITIZER_FLAGS -fsanitize=thread -fno-omit-frame-pointer)
    list(APPEND ZEN_SANITIZER_ENV "TSAN_OPTIONS=halt_on_error=1")

elseif(ZEN_SANITIZER STREQUAL "leak")
    if(NOT (_zen_is_gcc OR _zen_is_clang))
        message(FATAL_ERROR "leak sanitizer requires GCC or Clang")
    endif()
    if(APPLE)
        message(FATAL_ERROR "standalone leak sanitizer is not supported on macOS; use ASAN instead")
    endif()
    list(APPEND ZEN_SANITIZER_FLAGS -fsanitize=leak)
    list(APPEND ZEN_SANITIZER_ENV "LSAN_OPTIONS=halt_on_error=1")

elseif(ZEN_SANITIZER STREQUAL "integer")
    if(NOT _zen_is_clang)
        message(FATAL_ERROR "integer sanitizer is a Clang-only extension")
    endif()
    list(APPEND ZEN_SANITIZER_FLAGS -fsanitize=integer -fno-sanitize-recover=all)
    list(APPEND ZEN_SANITIZER_ENV "UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1")

elseif(ZEN_SANITIZER STREQUAL "cfi")
    if(NOT _zen_is_clang)
        message(FATAL_ERROR "CFI is a Clang-only extension")
    endif()
    if(APPLE)
        message(FATAL_ERROR "CFI needs LTO/lld in this setup and is disabled on macOS")
    endif()
    find_program(ZEN_LLD_EXECUTABLE ld.lld)
    if(ZEN_LLD_EXECUTABLE)
        # Linker selection is a link-only flag: passing it to the compiler
        # triggers -Wunused-command-line-argument under -Werror.
        list(APPEND ZEN_SANITIZER_LINK_FLAGS -fuse-ld=lld)
    endif()
    list(APPEND ZEN_SANITIZER_FLAGS
        -fsanitize=cfi -flto -fvisibility=hidden -fno-sanitize-recover=all)
    list(APPEND ZEN_SANITIZER_LINK_FLAGS -flto)

elseif(ZEN_SANITIZER STREQUAL "hwaddress")
    if(NOT _zen_is_clang OR NOT _zen_is_aarch64 OR APPLE)
        message(FATAL_ERROR "HWASAN requires Clang on 64-bit ARM Linux")
    endif()
    find_program(ZEN_LLD_EXECUTABLE ld.lld)
    if(ZEN_LLD_EXECUTABLE)
        # Link-only flag; see the CFI branch above.
        list(APPEND ZEN_SANITIZER_LINK_FLAGS -fuse-ld=lld)
    endif()
    list(APPEND ZEN_SANITIZER_FLAGS -fsanitize=hwaddress -fno-omit-frame-pointer)
    list(APPEND ZEN_SANITIZER_ENV "HWASAN_OPTIONS=halt_on_error=1")

elseif(ZEN_SANITIZER STREQUAL "shadow-call-stack")
    if(NOT _zen_is_clang OR NOT _zen_is_aarch64 OR APPLE)
        message(FATAL_ERROR "shadow call stack requires Clang on 64-bit ARM Linux")
    endif()
    list(APPEND ZEN_SANITIZER_FLAGS -fsanitize=shadow-call-stack)

else()
    message(FATAL_ERROR
        "Unknown ZEN_SANITIZER='${ZEN_SANITIZER}'. Valid values: none, address, "
        "undefined, thread, leak, integer, cfi, hwaddress, shadow-call-stack")
endif()

# Make sure the toolchain can actually compile *and link* with the requested
# sanitizer (the runtime may be missing even when the driver accepts the flag).
if(NOT ZEN_SANITIZER STREQUAL "none")
    string(REPLACE ";" " " _zen_required_flags "${ZEN_SANITIZER_FLAGS};${ZEN_SANITIZER_LINK_FLAGS}")
    set(CMAKE_REQUIRED_FLAGS "${_zen_required_flags}")
    unset(ZEN_SANITIZER_LINK_CHECK CACHE)
    check_c_source_compiles("int main(void) { volatile int value = 0; return value; }"
                            ZEN_SANITIZER_LINK_CHECK)
    unset(CMAKE_REQUIRED_FLAGS)
    if(NOT ZEN_SANITIZER_LINK_CHECK)
        message(FATAL_ERROR
            "ZEN_SANITIZER=${ZEN_SANITIZER} is not usable with ${CMAKE_C_COMPILER_ID} "
            "on ${CMAKE_SYSTEM_PROCESSOR} (flags: ${_zen_required_flags})")
    endif()
endif()

message(STATUS "zen sanitizer: ${ZEN_SANITIZER}${_zen_required_flags}")
