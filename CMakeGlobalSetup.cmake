function(setup_binary_locations)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endfunction()

function(setup_solution_folders)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endfunction()

function (setup_multicore_compilation)
    # This is only for Visual Studio. On Linux we should pass -j$(nproc) into make
    if(MSVC)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    endif()
endfunction()
