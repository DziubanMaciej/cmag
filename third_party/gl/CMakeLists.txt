find_package(OpenGL REQUIRED GLOBAL)
target_include_directories(OpenGL::GL INTERFACE ${CMAKE_CURRENT_LIST_DIR})
