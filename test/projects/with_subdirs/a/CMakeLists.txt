file(WRITE file.cpp "int main()")
add_library(LibA STATIC file.cpp)
