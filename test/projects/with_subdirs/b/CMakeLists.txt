file(WRITE file.cpp "int main()")
add_library(LibB STATIC file.cpp)

add_subdirectory(c)
add_subdirectory(d)
add_subdirectory(e)

target_link_libraries(LibB PUBLIC LibC)
target_link_libraries(LibB PRIVATE LibD)
target_link_libraries(LibB INTERFACE LibE)
