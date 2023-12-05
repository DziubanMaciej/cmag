function(target_common_setup TARGET_NAME)
    set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)

    if (MSVC)
        target_compile_definitions(${TARGET_NAME} PRIVATE -DNOMINMAX -DWIN32_LEAN_AND_MEAN -D_CRT_SECURE_NO_WARNINGS)
    endif ()

    target_compile_features(${TARGET_NAME} PRIVATE cxx_std_17)

    target_compile_options(${TARGET_NAME} PRIVATE
            $<$<CXX_COMPILER_ID:MSVC>:/W4>
            $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
    )
endfunction()

function(target_add_sources TARGET_NAME)
    set(SOURCE_FILES ${ARGN})
    target_sources(${TARGET_NAME} PRIVATE ${SOURCE_FILES})
endfunction()

function(target_setup_vs_folders TARGET_NAME)
    get_target_property(SOURCES ${TARGET_NAME} SOURCES)
    get_target_property(SOURCE_DIR ${TARGET_NAME} SOURCE_DIR)
    source_group(TREE ${SOURCE_DIR} FILES ${SOURCES})
endfunction()

function(target_find_sources_and_add TARGET_NAME)
    file(GLOB SOURCE_FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/*.h
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

function (generate_header_from_text TARGET TEXT_FILE_NAME GENERATED_HEADER_NAME VARIABLE_NAME)
    set(INPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${TEXT_FILE_NAME}")

    # Double appearance of "generated" in path is intentional. It lets C++ code do includes like
    # #include "generated/something.h", making it obvious that used code is generated.
    set(GENERATE_DIR_ROOT "${CMAKE_BINARY_DIR}/generated")
    set(OUTPUT_DIR "${GENERATE_DIR_ROOT}/generated")
    set(OUTPUT_PATH "${OUTPUT_DIR}/${GENERATED_HEADER_NAME}")

    # This wraps the text in a C++11 raw string literal. Note we have to backslash-escape some
    # characters (quotes, parentheses, backslashes, semicolons), so they are not interpreted
    # by CMake during add_custom_command
    set(PREFIX "const char *${VARIABLE_NAME} = R\\\"DELIMETER\\(")
    set(SUFFIX "\\)DELIMETER\\\"\\;")

    # The custom command specifies output file it generates. If we add this file as one of the
    # target's source files, then this target will start depending on the custom command.
    add_custom_command(
        OUTPUT "${OUTPUT_PATH}"
        COMMAND ${CMAKE_COMMAND} ARGS -E echo "${PREFIX}" > "${OUTPUT_PATH}"
        COMMAND ${CMAKE_COMMAND} ARGS -E cat "${INPUT_PATH}" >> "${OUTPUT_PATH}"
        COMMAND ${CMAKE_COMMAND} ARGS -E echo "${SUFFIX}" >> "${OUTPUT_PATH}"
        MAIN_DEPENDENCY "${INPUT_PATH}"
        DEPENDS "${INPUT_PATH}"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        COMMENT "Generating ${OUTPUT_PATH}"
    )
    target_sources(${TARGET} PRIVATE "${OUTPUT_PATH}")

    # Since this is a header file, we setup an include directory.
    target_include_directories(${TARGET} PRIVATE "${GENERATE_DIR_ROOT}")
endfunction()
