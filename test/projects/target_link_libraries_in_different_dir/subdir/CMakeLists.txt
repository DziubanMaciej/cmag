# This is meant to trigger weird syntax documented in
# https://cmake.org/cmake/help/latest/prop_tgt/LINK_LIBRARIES.html
#
# ::@(directory-id);...;::@
file(WRITE file.cpp "")
add_library(LibA STATIC file.cpp)
target_link_libraries(MainLib PUBLIC LibA)

# This is meant to trigger both weird syntax with directory-id and
# also $<LINK_ONLY>
file(WRITE file.cpp "")
add_library(LibB STATIC file.cpp)
target_link_libraries(MainLib PRIVATE LibB)
