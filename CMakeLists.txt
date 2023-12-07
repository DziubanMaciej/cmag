cmake_minimum_required(VERSION 3.15.0)
project(cmag)

# Option settable by user
set(CMAG_BUILD_BROWSER ON CACHE BOOL "If enabled, cmag-browser, a QT application for visualizing cmag projects will be built.")
set(CMAG_BUILD_TESTS ON CACHE BOOL "If enabled, tests for cmag will be built.")

# Configure some global options
include(CMakeGlobalSetup.cmake)
setup_binary_locations()
setup_solution_folders()
setup_multicore_compilation()
setup_static_crt()

# Import utils
include(CMakeUtils.cmake)

# Add actual code
add_subdirectory(third_party)
add_subdirectory(cmag_core)
add_subdirectory(cmag_exe)
if (CMAG_BUILD_BROWSER)
    add_subdirectory(cmag_browser)
endif()
if (CMAG_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
