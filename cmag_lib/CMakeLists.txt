add_library(cmag_lib STATIC CMakeLists.txt)
target_common_setup(cmag_lib)
target_find_sources_and_add(cmag_lib)
target_setup_vs_folders(cmag_lib)
target_include_directories(cmag_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(cmag_lib PUBLIC nlohmann_json::nlohmann_json)

add_subdirectories()
