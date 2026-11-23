zlang_add_test_module(spec)
target_link_libraries(zlang_spec_tests PRIVATE zlang::compiler_core)
