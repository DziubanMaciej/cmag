file(WRITE file.cpp "int main()")
add_library(LibE STATIC file.cpp)
target_include_directories(LibE PUBLIC /DirE)