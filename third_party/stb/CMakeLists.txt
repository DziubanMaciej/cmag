add_library(Stb INTERFACE)
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.19")
    # This call doesn't change anything in the buildsystem, but makes the library show up in IDEs.
    target_sources(Stb INTERFACE ${CMAKE_CURRENT_LIST_DIR}/stb/stb_image.h)
    set_target_properties(Stb PROPERTIES FOLDER ThirdParty)
endif()
target_include_directories(Stb INTERFACE ${CMAKE_CURRENT_LIST_DIR})
