if (CMAG_BUILD_TESTS)
    set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    add_subdirectory(googletest)
    set_target_properties(gtest gtest_main gmock gmock_main PROPERTIES FOLDER ThirdParty)
endif()

add_subdirectory(json)
set_target_properties(nlohmann_json PROPERTIES FOLDER ThirdParty)
