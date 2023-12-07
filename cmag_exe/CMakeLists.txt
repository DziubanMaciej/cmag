add_executable(cmag)
target_common_setup(cmag)
target_find_sources_and_add(cmag)
target_include_directories(cmag PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(cmag PRIVATE cmag_core)
add_subdirectories()
target_setup_vs_folders(cmag)

target_compile_definitions(cmag PUBLIC -DCONFIG=$<CONFIG>)