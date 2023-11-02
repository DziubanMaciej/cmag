cmake_minimum_required(VERSION 3.10.0)
project(WithSubdirs)
file(WRITE file.cpp "int main()")

add_executable(Executable file.cpp)
add_subdirectory(a)
add_subdirectory(b)
target_link_libraries(Executable PUBLIC LibA)
target_link_libraries(Executable PUBLIC LibB)
