cmake_minimum_required(VERSION 3.10.0)
project(Aliases)
set(CMAKE_SUPPRESS_REGENERATION true)
file(WRITE main.cpp "int main()")

add_library(LibA main.cpp)
add_library(Alias::LibA::1 ALIAS LibA)
add_library(Alias::LibA::2 ALIAS LibA)

add_library(LibB main.cpp)
add_library(Alias::LibB::1 ALIAS LibB)
add_library(Alias::LibB::2 ALIAS LibB)

add_executable(Exe0 main.cpp)
target_link_libraries(Exe0 PRIVATE LibA)
target_link_libraries(Exe0 PRIVATE LibB)

add_executable(Exe1 main.cpp)
add_executable(Alias::Exe1 ALIAS Exe1)
target_link_libraries(Exe1 PRIVATE Alias::LibA::1)
target_link_libraries(Exe1 PRIVATE Alias::LibB::1)

add_executable(Exe2 main.cpp)
add_executable(Alias::Exe2 ALIAS Exe2)
target_link_libraries(Exe2 PRIVATE Alias::LibA::2)
target_link_libraries(Exe2 PRIVATE Alias::LibB::2)

