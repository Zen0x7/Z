cmake_minimum_required(VERSION 3.21)

project(zlang_consumer LANGUAGES C)

find_package(zlang REQUIRED CONFIG)

add_executable(zlang_consumer main.c)
target_link_libraries(zlang_consumer PRIVATE zlang::zlang)
target_compile_features(zlang_consumer PRIVATE c_std_99)
