cmake_minimum_required(VERSION 3.21)

project(zen_consumer LANGUAGES C)

find_package(zen REQUIRED CONFIG)

add_executable(zen_consumer main.c)
target_link_libraries(zen_consumer PRIVATE zen::zen)
target_compile_features(zen_consumer PRIVATE c_std_99)
