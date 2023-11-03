cmake_minimum_required(VERSION 3.10.0)
project(Simple)
set(CMAKE_SUPPRESS_REGENERATION true)
file(WRITE main.cpp "int main()")

add_library(Library main.cpp)
target_link_directories(Library PUBLIC $<IF:$<CONFIG:Debug>,/D,/R>)
target_link_libraries(Library PUBLIC $<IF:$<CONFIG:Debug>,D,R>)
target_include_directories(Library PUBLIC $<IF:$<CONFIG:Debug>,/D,/R>)
target_compile_definitions(Library PUBLIC -DVALUE=$<IF:$<CONFIG:Debug>,D,R>)
target_compile_options(Library PUBLIC $<IF:$<CONFIG:Debug>,D,R>)
set_target_properties(Library PROPERTIES CUSTOM_PROP $<IF:$<CONFIG:Debug>,D,R>) # Should not expand
