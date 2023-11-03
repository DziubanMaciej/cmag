cmake_minimum_required(VERSION 3.10.0)
project(AllTypes)
set(CMAKE_SUPPRESS_REGENERATION true)
file(WRITE file.cpp "int main()")

add_library(StaticLib STATIC file.cpp)

add_library(ModuleLib MODULE file.cpp)

add_library(SharedLib SHARED file.cpp)
add_dependencies(SharedLib StaticLib)

add_library(ObjectLib OBJECT file.cpp)

add_library(InterfaceLib INTERFACE)
target_link_libraries(InterfaceLib INTERFACE SharedLib)

add_executable(Executable file.cpp)
target_link_libraries(Executable PRIVATE
    StaticLib
    # ModuleLib
    SharedLib
    ObjectLib
    InterfaceLib
)
