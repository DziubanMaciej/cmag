cmake_minimum_required(VERSION 3.10.0)
project(Simple)
set(CMAKE_SUPPRESS_REGENERATION true)
file(WRITE main.cpp "int main()")

add_executable(Exe main.cpp)
target_link_libraries(Exe PRIVATE external_lib)
target_include_directories(Exe PRIVATE Dir)
target_compile_options(Exe PUBLIC Option$<CONFIG>)
set_target_properties(Exe PROPERTIES MY_CUSTOM_PROP1 customValue1)
set_target_properties(Exe PROPERTIES MY_CUSTOM_PROP2 customValue2)
set_target_properties(Exe PROPERTIES MY_CUSTOM_PROP3 customValue3)
