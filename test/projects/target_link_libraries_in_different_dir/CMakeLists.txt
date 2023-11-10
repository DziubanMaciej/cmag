cmake_minimum_required(VERSION 3.13.0)
project(TargetLinkLibrariesInDifferentDir)

file(WRITE file.cpp "")
add_library(MainLib STATIC file.cpp)

add_subdirectory(subdir)
