file(WRITE file.cpp "int main()")
add_library(LibC STATIC file.cpp)
target_include_directories(LibC PUBLIC /DirC)
