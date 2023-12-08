# -----------------------------CMAG POSTAMBLE BEGIN-------------------------------------------
cmake_minimum_required(VERSION 3.9.0)





# -------------------------------------------------------------------- Utilities for JSON assembling
macro(json_append_line OUT_VARIABLE LINE INDENT)
    string(APPEND ${OUT_VARIABLE} "${INDENT}${LINE}\n")
endmacro()

function(json_append_key_value OUT_VARIABLE KEY VALUE INDENT)
    json_append_key_value_unquoted(${OUT_VARIABLE} ${KEY} "\"${VALUE}\"" ${INDENT})

    set(${OUT_VARIABLE} ${${OUT_VARIABLE}} PARENT_SCOPE)
endfunction()

function(json_append_key_value_unquoted OUT_VARIABLE KEY VALUE INDENT)
    set(LINE "\"${KEY}\": ${VALUE},")
    json_append_line(${OUT_VARIABLE} ${LINE} ${INDENT})

    set(${OUT_VARIABLE} ${${OUT_VARIABLE}} PARENT_SCOPE)
endfunction()

macro(json_strip_trailing_comma)
    string(REGEX REPLACE ",\n$" "\n" ${OUT_VARIABLE} "${${OUT_VARIABLE}}")
endmacro()

function(json_append_object_begin OUT_VARIABLE NAME INDENT)
    string(APPEND ${OUT_VARIABLE} "${INDENT}\"${NAME}\": {\n")
    set(${OUT_VARIABLE} ${${OUT_VARIABLE}} PARENT_SCOPE)
endfunction()

function(json_append_object_end OUT_VARIABLE INDENT)
    json_strip_trailing_comma()
    string(APPEND ${OUT_VARIABLE} "${INDENT}},\n")
    set(${OUT_VARIABLE} ${${OUT_VARIABLE}} PARENT_SCOPE)
endfunction()

function(json_append_array_end OUT_VARIABLE INDENT)
    json_strip_trailing_comma()
    string(APPEND ${OUT_VARIABLE} "${INDENT}],\n")
    set(${OUT_VARIABLE} ${${OUT_VARIABLE}} PARENT_SCOPE)
endfunction()

# -------------------------------------------------------------------- Assembling JSON for .cmag-targets file
macro(json_append_target_property OUT_VARIABLE TGT NAME PROPERTY INDENT GENEX_EVAL)
    set(VALUE "$<TARGET_PROPERTY:${TGT},${PROPERTY}>")
    if(${GENEX_EVAL})
        set(VALUE "$<TARGET_GENEX_EVAL:${TGT},${VALUE}>")
    endif()

    # Escape quotes in property values
    set(VALUE "$<LIST:TRANSFORM,${VALUE},REPLACE,\",\\\\\\\\\">")

    json_append_key_value(${OUT_VARIABLE} "${NAME}" "${VALUE}" ${INDENT})
endmacro()

function (json_append_target_properties OUT_VARIABLE TGT INDENT INDENT_INCREMENT)
    set(INNER_INDENT "${INDENT}${INDENT_INCREMENT}")
    set(INNER_INNER_INDENT "${INDENT}${INDENT_INCREMENT}${INDENT_INCREMENT}")

    # This is a list of properties which may contain generator expressions (genexes) after querying
    # with $<TARGET_PROPERTY>. We have to manually wrap them with genex eval to resolve to actual
    # values.
    set(GENEXABLE_PROPERTIES
        LINK_LIBRARIES
        INTERFACE_LINK_LIBRARIES
    )

    # This is a list of all properties we will be querying. We will treat them differently depending
    # whether they are genexable or not.
    set(PROPERTIES_TO_QUERY
        LINK_DIRECTORIES
        LINK_LIBRARIES
        INCLUDE_DIRECTORIES
        COMPILE_DEFINITIONS
        COMPILE_FEATURES
        COMPILE_OPTIONS
        COMPILE_FLAGS
        LINK_OPTIONS
        INTERFACE_LINK_DIRECTORIES
        INTERFACE_LINK_LIBRARIES
        INTERFACE_INCLUDE_DIRECTORIES
        INTERFACE_COMPILE_DEFINITIONS
        INTERFACE_COMPILE_FEATURES
        INTERFACE_COMPILE_OPTIONS
        INTERFACE_LINK_OPTIONS
        SOURCES
        MANUALLY_ADDED_DEPENDENCIES
        FOLDER
        ${CMAG_EXTRA_TARGET_PROPERTIES}
    )

    # Write non-genexable properties
    json_append_object_begin(${OUT_VARIABLE} "non_genexable" ${INDENT})
    foreach(PROP ${PROPERTIES_TO_QUERY})
        list(FIND GENEXABLE_PROPERTIES ${PROP} INDEX)
        if(NOT INDEX EQUAL -1)
            continue()
        endif()

        json_append_target_property(${OUT_VARIABLE} ${TGT} ${PROP} ${PROP} ${INNER_INDENT} FALSE FALSE)
    endforeach()
    json_append_object_end(${OUT_VARIABLE} ${INDENT})

    # Write genexable properties without evaluation
    json_append_object_begin(${OUT_VARIABLE} "genexable" ${INDENT})
    foreach(PROP ${PROPERTIES_TO_QUERY})
        list(FIND GENEXABLE_PROPERTIES ${PROP} INDEX)
        if(INDEX EQUAL -1)
            continue()
        endif()

        json_append_target_property(${OUT_VARIABLE} ${TGT} ${PROP} ${PROP} ${INNER_INDENT} FALSE FALSE)
    endforeach()
    json_append_object_end(${OUT_VARIABLE} ${INDENT})

    # Write genexable properties with evaluation
    json_append_object_begin(${OUT_VARIABLE} "genexable_evaled" ${INDENT})
    foreach(PROP ${PROPERTIES_TO_QUERY})
        list(FIND GENEXABLE_PROPERTIES ${PROP} INDEX)
        if(INDEX EQUAL -1)
            continue()
        endif()

        json_append_target_property(${OUT_VARIABLE} ${TGT} ${PROP} ${PROP} ${INNER_INDENT} TRUE FALSE)
    endforeach()
    json_append_object_end(${OUT_VARIABLE} ${INDENT})

    set(${OUT_VARIABLE} ${${OUT_VARIABLE}} PARENT_SCOPE)
endfunction()

function(json_append_target OUT_VARIABLE TGT CONFIG INDENT INDENT_INCREMENT)
    set(INNER_INDENT "${INDENT}${INDENT_INCREMENT}")
    set(INNER_INNER_INDENT "${INDENT}${INDENT_INCREMENT}${INDENT_INCREMENT}")
    set(INNER_INNER_INNER_INDENT "${INDENT}${INDENT_INCREMENT}${INDENT_INCREMENT}${INDENT_INCREMENT}")
    set(INNER_INNER_INNER_INNER_INDENT "${INDENT}${INDENT_INCREMENT}${INDENT_INCREMENT}${INDENT_INCREMENT}${INDENT_INCREMENT}")

    json_append_object_begin(${OUT_VARIABLE} "${TGT}" ${INDENT})
    json_append_target_property(${OUT_VARIABLE} ${TGT} type TYPE ${INNER_INDENT} FALSE FALSE)
    json_append_target_property(${OUT_VARIABLE} ${TGT} listDir CMAG_CMAKE_LIST_DIR ${INNER_INDENT} FALSE FALSE)
    json_append_object_begin(${OUT_VARIABLE} "configs" ${INNER_INDENT})
    json_append_object_begin(${OUT_VARIABLE} "${CONFIG}" ${INNER_INNER_INDENT})

    json_append_target_properties(${OUT_VARIABLE} ${TGT} ${INNER_INNER_INNER_INNER_INDENT} ${INDENT_INCREMENT})

    json_append_object_end(${OUT_VARIABLE} ${INNER_INNER_INDENT})
    json_append_object_end(${OUT_VARIABLE} ${INNER_INDENT})
    json_append_object_end(${OUT_VARIABLE} ${INDENT})

    set(${OUT_VARIABLE} ${${OUT_VARIABLE}} PARENT_SCOPE)
endfunction()

function (get_all_targets OUT_VARIABLE DIR)
    # Call for this directory
    get_property(TARGETS_IN_CURRENT_DIR DIRECTORY ${DIR} PROPERTY BUILDSYSTEM_TARGETS)
    list(APPEND ${OUT_VARIABLE} ${TARGETS_IN_CURRENT_DIR})

    # Set a custom property, so we know where this target was defined
    set_target_properties(${TARGETS_IN_CURRENT_DIR} PROPERTIES CMAG_CMAKE_LIST_DIR ${DIR})

    # Call for subdirectories recursively
    get_property(SUBDIRS DIRECTORY ${DIR} PROPERTY SUBDIRECTORIES)
    foreach (SUBDIR ${SUBDIRS})
        get_all_targets(${OUT_VARIABLE} ${SUBDIR})
    endforeach ()

    # Propagate to outer scope
    set(${OUT_VARIABLE} ${${OUT_VARIABLE}} PARENT_SCOPE)
endfunction()

function(json_append_targets OUT_VARIABLE CONFIG INDENT INDENT_INCREMENT)
    set(INNER_INDENT "${INDENT}${INDENT_INCREMENT}")

    get_all_targets(ALL_TARGETS ${CMAKE_CURRENT_SOURCE_DIR})

    json_append_line(${OUT_VARIABLE} "{" ${INDENT})
    foreach(TGT ${ALL_TARGETS})
        json_append_target(${OUT_VARIABLE} ${TGT} ${CONFIG} ${INNER_INDENT} ${INDENT_INCREMENT})
    endforeach()
    json_strip_trailing_comma()
    json_append_line(${OUT_VARIABLE} "}" ${INDENT})

    set(${OUT_VARIABLE} ${${OUT_VARIABLE}} PARENT_SCOPE)
endfunction()





# -------------------------------------------------------------------- Assembling JSON for .cmag-globals file
function (json_append_lists_files OUT_VARIABLE DIR INDENT INDENT_INCREMENT)
    set(INNER_INDENT "${INDENT}${INDENT_INCREMENT}")

    # Get immediate subdirectories
    get_property(SUBDIRS DIRECTORY ${DIR} PROPERTY SUBDIRECTORIES)

    # Write current dir and its immediate subdirectories
    if ("${SUBDIRS}d" STREQUAL "d")
        string(APPEND ${OUT_VARIABLE} "${INDENT}\"${DIR}\": [],\n")
    else ()
        string(APPEND ${OUT_VARIABLE} "${INDENT}\"${DIR}\": [\n")
        foreach (SUBDIR ${SUBDIRS})
            json_append_line(${OUT_VARIABLE} "\"${SUBDIR}\"," ${INNER_INDENT})
        endforeach ()
        json_append_array_end(${OUT_VARIABLE} ${INDENT})
    endif()

    # Execute recursively
    foreach (SUBDIR ${SUBDIRS})
        json_append_lists_files(${OUT_VARIABLE} ${SUBDIR} ${INDENT} ${INDENT_INCREMENT})
    endforeach ()

    # Propagate to outer scope
    set(${OUT_VARIABLE} ${${OUT_VARIABLE}} PARENT_SCOPE)
endfunction()

function(json_append_globals OUT_VARIABLE SELECTED_CONFIG INDENT INDENT_INCREMENT)
    set(INNER_INDENT "${INDENT}${INDENT_INCREMENT}")
    set(INNER_INNER_INDENT "${INDENT}${INDENT_INCREMENT}${INDENT_INCREMENT}")

    json_append_line(${OUT_VARIABLE} "{" ${INDENT})
    json_append_key_value_unquoted(${OUT_VARIABLE} darkMode true ${INNER_INDENT})
    json_append_key_value(${OUT_VARIABLE} selectedConfig "${SELECTED_CONFIG}" ${INNER_INDENT})
    json_append_key_value(${OUT_VARIABLE} cmagVersion "${CMAG_VERSION}" ${INNER_INDENT})
    json_append_key_value(${OUT_VARIABLE} cmakeVersion "${CMAKE_VERSION}" ${INNER_INDENT})
    json_append_key_value(${OUT_VARIABLE} cmakeProjectName "${CMAKE_PROJECT_NAME}" ${INNER_INDENT})
    json_append_key_value(${OUT_VARIABLE} cmagProjectName "${CMAG_PROJECT_NAME}" ${INNER_INDENT})
    json_append_key_value(${OUT_VARIABLE} sourceDir "${CMAKE_SOURCE_DIR}" ${INNER_INDENT})
    json_append_key_value(${OUT_VARIABLE} buildDir "${CMAKE_BINARY_DIR}" ${INNER_INDENT})
    json_append_key_value(${OUT_VARIABLE} generator "${CMAKE_GENERATOR}" ${INNER_INDENT})
    json_append_key_value(${OUT_VARIABLE} compilerId "${CMAKE_CXX_COMPILER_ID}" ${INNER_INDENT})
    json_append_key_value(${OUT_VARIABLE} compilerVersion "${CMAKE_CXX_COMPILER_VERSION}" ${INNER_INDENT})
    json_append_key_value(${OUT_VARIABLE} os "${CMAKE_SYSTEM_NAME}" ${INNER_INDENT})

    json_append_object_begin(${OUT_VARIABLE} "listDirs" ${INNER_INDENT})
    json_append_lists_files(${OUT_VARIABLE} ${CMAKE_CURRENT_SOURCE_DIR} ${INNER_INNER_INDENT} ${INDENT_INCREMENT})
    json_append_object_end(${OUT_VARIABLE} ${INNER_INDENT})

    json_strip_trailing_comma()
    json_append_line(${OUT_VARIABLE} "}" ${INDENT})

    set(${OUT_VARIABLE} ${${OUT_VARIABLE}} PARENT_SCOPE)
endfunction()





# -------------------------------------------------------------------- Assembling JSON for .cmag-targets-list file
function(json_append_configs OUT_VARIABLE CONFIGS INDENT INDENT_INCREMENT)
    set(INNER_INDENT "${INDENT}${INDENT_INCREMENT}")

    json_append_line(${OUT_VARIABLE} "[" ${INDENT})
    foreach(CONFIG ${CONFIGS})
        set(LINE "${CMAG_PROJECT_NAME}_${CONFIG}.cmag-targets")
        json_append_line(${OUT_VARIABLE} "\"${LINE}\"," ${INNER_INDENT})
    endforeach()
    json_strip_trailing_comma()
    json_append_line(${OUT_VARIABLE} "]" ${INDENT})

    set(${OUT_VARIABLE} ${${OUT_VARIABLE}} PARENT_SCOPE)
endfunction()





# -------------------------------------------------------------------- Main code
# Verify project name is set.
if ("${CMAG_PROJECT_NAME}d" STREQUAL "d")
    message(FATAL_ERROR "cmag: CMAG_PROJECT_NAME should be set by cmag")
endif()

# Handle single-config and multi-config generators differently.
get_property(CMAG_IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(CMAG_IS_MULTI_CONFIG)
    set(CMAG_CONFIGS "${CMAKE_CONFIGURATION_TYPES}")
    set(CMAG_CONFIG "$<CONFIG>")
    list(GET CMAKE_CONFIGURATION_TYPES 0 CMAG_CONFIG_DEFAULT)
else()
    # Work out current config. It can be unspecified, in which cas we'll name it as "Default".
    # Note that $<CONFIG> generator expression doesn't work if build type isn't explicitly
    # specified, so we have use this placeholder.
    set(CMAG_CONFIG ${CMAKE_BUILD_TYPE})
    if ("${CMAG_CONFIG}d" STREQUAL "d")
        set(CMAG_CONFIG Default)
    endif()

    set(CMAG_CONFIGS "${CMAG_CONFIG}")
    set(CMAG_CONFIG_DEFAULT "${CMAG_CONFIG}")
endif()

# Write configs list
json_append_configs(CONFIGS_JSON "${CMAG_CONFIGS}" "  " "  ")
set(TARGETS_LIST_FILE "${CMAKE_BINARY_DIR}/${CMAG_PROJECT_NAME}.cmag-targets-list")
file(WRITE "${TARGETS_LIST_FILE}" "${CONFIGS_JSON}")

# Write global settings
json_append_globals(GLOBALS_JSON "${CMAG_CONFIG_DEFAULT}" "  " "  ")
set(GLOBALS_FILE "${CMAKE_BINARY_DIR}/${CMAG_PROJECT_NAME}.cmag-globals")
file(WRITE "${GLOBALS_FILE}" "${GLOBALS_JSON}")

# Write per-config targets
json_append_targets(TARGETS_JSON "${CMAG_CONFIG}" "  " "  ")
set(TARGETS_LIST_FILE "${CMAKE_BINARY_DIR}/${CMAG_PROJECT_NAME}_${CMAG_CONFIG}.cmag-targets")
file(GENERATE OUTPUT "${TARGETS_LIST_FILE}" CONTENT "${TARGETS_JSON}")

message(STATUS "cmag: generating file ${TARGETS_LIST_FILE}")
message(STATUS "cmag: generating file ${GLOBALS_FILE}")
message(STATUS "cmag: generating file ${TARGETS_LIST_FILE}")

if (CMAG_JSON_DEBUG)
    set(TARGETS_LIST_DEBUG_FILE "${CMAKE_BINARY_DIR}/${CMAG_PROJECT_NAME}.cmag-targets.debug")
    message(STATUS "cmag: writing debug file ${TARGETS_LIST_DEBUG_FILE}")
    file(WRITE "${TARGETS_LIST_DEBUG_FILE}" "${TARGETS_JSON}")
endif()

# -----------------------------CMAG POSTAMBLE END---------------------------------------------