include(ZlangTesting)

# Adding a module is one line here plus tests/<module>/{CMakeLists.txt,
# test_cases.h, <module>_tests.c}. No other module is touched.
add_subdirectory(version)
add_subdirectory(testing)
add_subdirectory(spec)
