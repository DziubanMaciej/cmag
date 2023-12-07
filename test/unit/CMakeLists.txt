add_executable(cmag_unit_tests)
target_common_setup(cmag_unit_tests)
target_find_sources_and_add(cmag_unit_tests)
target_link_libraries(cmag_unit_tests PRIVATE cmag_core gtest)
set_target_properties(cmag_unit_tests PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "--gtest_filter=*")
add_subdirectories()
target_setup_vs_folders(cmag_unit_tests)

add_test(NAME UnitTests COMMAND cmag_unit_tests)
