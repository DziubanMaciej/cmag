cmake_minimum_required(VERSION 3.10.0)
project(Simple)

file(WRITE main.cpp "int main()")

add_executable(Exe main.cpp)
target_link_libraries(Exe PRIVATE external_lib)
target_include_directories(Exe PRIVATE Dir)
