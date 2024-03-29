add_library(cmag_core STATIC CMakeLists.txt)
target_common_setup(cmag_core)
target_find_sources_and_add(cmag_core)
target_include_directories(cmag_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(cmag_core PUBLIC nlohmann_json::nlohmann_json)
add_subdirectories()
target_setup_vs_folders(cmag_core)
setup_vs_folders_for_interface_source(nlohmann_json "external" FROM_PATHS nlohmann_json.natvis)

set(CMAG_BROWSER_BINARY_NAME "cmag_browser")
if (WIN32)
    set(CMAG_BROWSER_BINARY_NAME "${CMAG_BROWSER_BINARY_NAME}.exe")
endif()
target_compile_definitions(cmag_core PRIVATE -DCMAG_BROWSER_BINARY_NAME="${CMAG_BROWSER_BINARY_NAME}")

header_pack_generate(TEXT cmag_core shim/postamble.cmake postamble.h VARIABLE postamble OUTPUT_SOURCE_GROUP generated)
header_pack_generate(TEXT cmag_core ../doc/cmag_cli.txt cmag_cli.h VARIABLE cmagCli OUTPUT_SOURCE_GROUP generated)
header_pack_generate(TEXT cmag_core ../doc/cmag_browser_cli.txt cmag_browser_cli.h VARIABLE cmagBrowserCli OUTPUT_SOURCE_GROUP generated)
