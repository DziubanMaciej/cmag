add_executable(cmag_os_tests)
target_common_setup(cmag_os_tests)
target_find_sources_and_add(cmag_os_tests)
target_link_libraries(cmag_os_tests PRIVATE cmag_core gtest)
target_compile_definitions(cmag_os_tests PRIVATE
    -DSRC_PROJECTS_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/../projects"
    -DDST_PROJECTS_ROOT="${CMAKE_BINARY_DIR}/os_tests_projects/"
)
set_target_properties(cmag_os_tests PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "--gtest_filter=*")
add_subdirectories()
target_setup_vs_folders(cmag_os_tests)
setup_vs_folders_for_interface_source(nlohmann_json "external" FROM_PATHS nlohmann_json.natvis)

add_test(NAME OsTests COMMAND cmag_os_tests)
