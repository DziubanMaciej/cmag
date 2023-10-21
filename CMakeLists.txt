cmake_minimum_required(VERSION 3.10.0)
project(cmag)

# Configure some global options
include(CMakeGlobalSetup.cmake)
setup_binary_locations()
setup_solution_folders()
setup_multicore_compilation()

# Import utils
include(CMakeUtils.cmake)

# Add actual code
add_subdirectory(cmag_lib)
add_subdirectory(cmag_exe)
add_subdirectory(third_party)
enable_testing()
add_subdirectory(test)
