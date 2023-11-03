file(WRITE file.cpp "int main()")
add_library(LibD STATIC file.cpp)
target_include_directories(LibD PUBLIC /DirD)