function(target_common_setup TARGET_NAME)
    set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)

    if (MSVC)
        target_compile_definitions(${TARGET_NAME} PRIVATE -DNOMINMAX -DWIN32_LEAN_AND_MEAN -D_CRT_SECURE_NO_WARNINGS)
    endif ()

    target_compile_features(${TARGET_NAME} PRIVATE cxx_std_17)

    target_compile_options(${TARGET_NAME} PRIVATE
            $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
            $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
    )
endfunction()

function(target_add_sources TARGET_NAME)
    set(SOURCE_FILES ${ARGN})
    target_sources(${TARGET_NAME} PRIVATE ${SOURCE_FILES})
endfunction()

function(target_setup_vs_folders TARGET_NAME)
    get_target_property(SOURCES ${TARGET_NAME} SOURCES)
    set(NON_GENERATED_SOURCES)
    foreach(FILE ${SOURCES})
        get_source_file_property(IS_GENERATED ${FILE} GENERATED)
        if (NOT IS_GENERATED OR IS_GENERATED STREQUAL "NOTFOUND")
            list(APPEND NON_GENERATED_SOURCES ${FILE})
        endif()
    endforeach()

    get_target_property(SOURCE_DIR ${TARGET_NAME} SOURCE_DIR)
    source_group(TREE ${SOURCE_DIR} FILES ${NON_GENERATED_SOURCES})
endfunction()

macro(setup_vs_folders_for_interface_source OTHER_TARGET_NAME GROUP_NAME)
    if(CMAKE_VERSION VERSION_LESS "3.19")
        return()
    endif()

    # Parse optional arguments
    set(OPTION_ARGS FROM_PROPERTY)
    set(VALUE_ARGS)
    set(MULTI_VALUE_ARGS FROM_PATHS)
    cmake_parse_arguments(ARG "${OPTION_ARGS}" "${VALUE_ARGS}" "${MULTI_VALUE_ARGS}" ${ARGN})

    if (${ARG_FROM_PROPERTY})
        get_target_property(SOURCES ${OTHER_TARGET_NAME} INTERFACE_SOURCES)
        source_group("${GROUP_NAME}" FILES "${SOURCES}")
    endif()

    if (NOT "${ARG_FROM_PATHS}d" STREQUAL "d")
        get_target_property(DIR ${OTHER_TARGET_NAME} SOURCE_DIR)
        foreach(SOURCE ${ARG_FROM_PATHS})
            source_group("${GROUP_NAME}" FILES "${DIR}/${SOURCE}")
        endforeach()
    endif()
endmacro()

function(target_find_sources_and_add TARGET_NAME)
    file(GLOB SOURCE_FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/*.cmake
            ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/*.h
            ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
    )
    target_add_sources(${TARGET_NAME} ${SOURCE_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt)
endfunction()

macro(add_subdirectories)
    if (UNIX)
        set(FORBIDDEN_DIR_NAMES "windows")
    elseif (WIN32)
        set(FORBIDDEN_DIR_NAMES "linux")
    else ()
        message(FATAL_ERROR "Unsupported OS")
    endif ()

    file(GLOB SUB_DIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)
    foreach (SUB_DIR ${SUB_DIRS})
        if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${SUB_DIR}/CMakeLists.txt)
            continue()
        endif ()

        list(FIND FORBIDDEN_DIR_NAMES ${SUB_DIR} INDEX)
        if (NOT INDEX EQUAL -1)
            continue()
        endif ()

        add_subdirectory(${SUB_DIR})
    endforeach ()
endmacro()
