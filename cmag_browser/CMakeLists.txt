add_executable(cmag_browser)
target_common_setup(cmag_browser)
target_find_sources_and_add(cmag_browser)
target_include_directories(cmag_browser PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(cmag_browser PRIVATE Imgui cmag_core glm)
add_subdirectories()
target_setup_vs_folders(cmag_browser)

install(TARGETS cmag_browser)
