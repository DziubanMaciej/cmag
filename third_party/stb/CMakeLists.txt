add_library(Stb INTERFACE stb/stb_image.h)
target_include_directories(Stb INTERFACE ${CMAKE_CURRENT_LIST_DIR})
set_target_properties(Stb PROPERTIES FOLDER ThirdParty)
