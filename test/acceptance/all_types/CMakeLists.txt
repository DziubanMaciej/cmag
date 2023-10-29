cmake_minimum_required(VERSION 3.10.0)
project(AllTypes)

# STATIC_LIBRARY
add_library(StaticLib STATIC file.cpp)

# MODULE_LIBRARY
add_library(ModuleLib MODULE file.cpp)

# SHARED_LIBRARY
add_library(SharedLib SHARED file.cpp)

# OBJECT_LIBRARY
add_library(ObjectLib OBJECT file.cpp)

# INTERFACE_LIBRARY
add_library(InterfaceLib INTERFACE)

# EXECUTABLE
add_executable(Executable file.cpp)
target_link_libraries(Executable PRIVATE
    StaticLib
    # ModuleLib
    SharedLib
    ObjectLib
    InterfaceLib
)
