if (CMAG_BUILD_TESTS)
    set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    add_subdirectory(googletest)
    set_target_properties(gtest gtest_main gmock gmock_main PROPERTIES FOLDER ThirdParty)
endif()

add_subdirectory(header_pack)
set_target_properties(header_pack PROPERTIES FOLDER ThirdParty)

if (CMAG_BUILD_BROWSER)
    add_subdirectory(gl)
    add_subdirectory(glm)
    set_target_properties(glm uninstall PROPERTIES FOLDER ThirdParty)

    set(GLFW_BUILD_DOCS OFF)
    set(GLFW_INSTALL OFF)
    add_subdirectory(glfw)
    set_target_properties(glfw update_mappings PROPERTIES FOLDER ThirdParty/GLFW)

    add_subdirectory(imgui)
endif()

add_subdirectory(json)
set_target_properties(nlohmann_json PROPERTIES FOLDER ThirdParty)
