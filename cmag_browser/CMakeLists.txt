add_executable(cmag_browser
        ui/target_graph/shapes.h
        ui/target_graph/shapes.cpp)
target_common_setup(cmag_browser)
target_find_sources_and_add(cmag_browser)
target_include_directories(cmag_browser PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(cmag_browser PRIVATE Imgui cmag_lib glm)
add_subdirectories()
target_setup_vs_folders(cmag_browser)
