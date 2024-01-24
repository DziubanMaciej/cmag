if (CMAG_BUILD_TESTS)
    set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    add_subdirectory(googletest)
    set_target_properties(gtest gtest_main gmock gmock_main PROPERTIES FOLDER ThirdParty)
endif()

add_subdirectory(header_pack)
set_target_properties(header_pack PROPERTIES FOLDER ThirdParty)

if (CMAG_BUILD_BROWSER)
    find_package(OpenGL REQUIRED)
    target_include_directories(OpenGL::GL INTERFACE ${CMAKE_CURRENT_LIST_DIR}/gl_headers)

    add_subdirectory(glm)
    target_compile_definitions(glm INTERFACE GLM_FORCE_SILENT_WARNINGS=1)
    set_target_properties(uninstall PROPERTIES FOLDER ThirdParty)

    set(GLFW_BUILD_DOCS OFF)
    set(GLFW_INSTALL OFF)
    add_subdirectory(glfw)
    set_target_properties(glfw update_mappings PROPERTIES FOLDER ThirdParty/GLFW)

    add_subdirectory(imgui)

    add_subdirectory(stb)
endif()

add_subdirectory(json)
