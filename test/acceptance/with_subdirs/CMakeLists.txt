cmake_minimum_required(VERSION 3.10.0)
project(WithSubdirs)

add_executable(Exe main.cpp)
add_subdirectory(subdir)
add_subdirectory(subdir2)
target_link_libraries(Exe PRIVATE MyOtherLib)
target_link_libraries(Exe PRIVATE MyOtherOtherLib)
