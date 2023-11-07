const char *postamble = R"DELIMETER(

# -----------------------------CMAG POSTAMBLE BEGIN-------------------------------------------
cmake_minimum_required(VERSION 3.9.0)





# -------------------------------------------------------------------- Utilities for JSON assembling
macro(json_append_line OUT_VARIABLE LINE INDENT)
    string(APPEND ${OUT_VARIABLE} "${INDENT}${LINE}\n")
endmacro()

function(json_append_line_comma OUT_VARIABLE LINE INDENT IS_LAST)
    if(NOT ${IS_LAST})
        set(LINE "${LINE},")
    endif()
    string(APPEND ${OUT_VARIABLE} "${INDENT}${LINE}\n")

    set(${OUT_VARIABLE} ${${OUT_VARIABLE}} PARENT_SCOPE)
endfunction()

function(json_append_key_value OUT_VARIABLE KEY VALUE INDENT IS_LAST)
    json_append_key_value_unquoted(${OUT_VARIABLE} ${KEY} "\"${VALUE}\"" ${INDENT} ${IS_LAST})

    set(${OUT_VARIABLE} ${${OUT_VARIABLE}} PARENT_SCOPE)
endfunction()

function(json_append_key_value_unquoted OUT_VARIABLE KEY VALUE INDENT IS_LAST)
    set(LINE "\"${KEY}\": ${VALUE}")
    if(NOT ${IS_LAST})
        set(LINE "${LINE},")
    endif()

    json_append_line(${OUT_VARIABLE} ${LINE} ${INDENT})

    set(${OUT_VARIABLE} ${${OUT_VARIABLE}} PARENT_SCOPE)
endfunction()





# -------------------------------------------------------------------- Assembling JSON for .cmag-targets file
macro(json_append_target_property OUT_VARIABLE TGT PROPERTY INDENT IS_LAST)
    json_append_target_named_property(${OUT_VARIABLE} ${TGT} ${PROPERTY} ${PROPERTY} ${INDENT} ${IS_LAST})
endmacro()

macro(json_append_target_named_property OUT_VARIABLE TGT NAME PROPERTY INDENT IS_LAST)
    set(VALUE "$<TARGET_PROPERTY:${TGT},${PROPERTY}>")
    if("${PROPERTY}" STREQUAL "LINK_LIBRARIES")
        set(VALUE "$<GENEX_EVAL:${VALUE}>")
    endif()
    if("${PROPERTY}" STREQUAL "INTERFACE_LINK_LIBRARIES")
        # TODO: this is incorrect. When a library have an PRIVATE dependency X, it will appear in
        # INTERFACE_LINK_LIBRARIES as $<LINK_ONLY:X> and will be resolved to "X" instead of "".
        # Possible WA: dump both non-genex-evaled and genex-evaled property, parse expressions with
        # $<LINK_ONLY> in the former, and remove them from the latter.
        set(VALUE "$<TARGET_GENEX_EVAL:${TGT},${VALUE}>")
    endif()

    # Escape quotes in property values
    set(VALUE "$<LIST:TRANSFORM,${VALUE},REPLACE,\",\\\\\\\\\">")

    json_append_key_value(${OUT_VARIABLE} "${NAME}" "${VALUE}" ${INDENT} ${IS_LAST})
endmacro()

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

function(json_append_target OUT_VARIABLE TGT CONFIG INDENT INDENT_INCREMENT IS_LAST)
    set(INNER_INDENT "${INDENT}${INDENT_INCREMENT}")
    set(INNER_INNER_INDENT "${INDENT}${INDENT_INCREMENT}${INDENT_INCREMENT}")
    set(INNER_INNER_INNER_INDENT "${INDENT}${INDENT_INCREMENT}${INDENT_INCREMENT}${INDENT_INCREMENT}")

    json_append_line(${OUT_VARIABLE} "\"${TGT}\": {" ${INDENT})
    json_append_target_named_property(${OUT_VARIABLE} ${TGT} type TYPE ${INNER_INDENT} FALSE)
    json_append_target_named_property(${OUT_VARIABLE} ${TGT} listDir CMAG_CMAKE_LIST_DIR ${INNER_INDENT} FALSE)

    json_append_line(${OUT_VARIABLE} "\"configs\": {" ${INNER_INDENT})
    json_append_line(${OUT_VARIABLE} "\"${CONFIG}\": {" ${INNER_INNER_INDENT})
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
        ${CMAG_EXTRA_TARGET_PROPERTIES}
    )
    foreach(PROP ${PROPERTIES_TO_QUERY})
        json_append_target_property(${OUT_VARIABLE} ${TGT} ${PROP} ${INNER_INNER_INNER_INDENT} FALSE)
    endforeach()
    json_append_target_property(${OUT_VARIABLE} ${TGT} MANUALLY_ADDED_DEPENDENCIES ${INNER_INNER_INNER_INDENT} TRUE)
    json_append_line_comma(${OUT_VARIABLE} "}" ${INNER_INNER_INDENT} TRUE)
    json_append_line_comma(${OUT_VARIABLE} "}" ${INNER_INDENT} TRUE)

    json_append_line_comma(${OUT_VARIABLE} "}" ${INDENT} ${IS_LAST})
    set(${OUT_VARIABLE} ${${OUT_VARIABLE}} PARENT_SCOPE)
endfunction()

function(json_append_targets OUT_VARIABLE CONFIG INDENT INDENT_INCREMENT)
    set(INNER_INDENT "${INDENT}${INDENT_INCREMENT}")

    json_append_line(${OUT_VARIABLE} "{" ${INDENT})

    get_all_targets(ALL_TARGETS ${CMAKE_CURRENT_SOURCE_DIR})

    list(LENGTH ALL_TARGETS LAST_TARGET_INDEX)
    math(EXPR LAST_TARGET_INDEX "${LAST_TARGET_INDEX} - 1")
    set(COUNTER 0)
    foreach(TGT IN LISTS ALL_TARGETS)
        if(LAST_TARGET_INDEX EQUAL COUNTER)
            set(IS_LAST TRUE)
        else()
            set(IS_LAST FALSE)
        endif()
        math(EXPR COUNTER "${COUNTER}+1")

        json_append_target(${OUT_VARIABLE} ${TGT} ${CONFIG} ${INNER_INDENT} ${INDENT_INCREMENT} ${IS_LAST})
    endforeach()

    json_append_line(${OUT_VARIABLE} "}" ${INDENT})

    set(${OUT_VARIABLE} ${${OUT_VARIABLE}} PARENT_SCOPE)
endfunction()





# -------------------------------------------------------------------- Assembling JSON for .cmag-globals file
function (json_append_lists_files OUT_VARIABLE PARENT_DIR DIR INDENT)
    # Append current dir to json
    if (NOT "${PARENT_DIR}d" STREQUAL "d")
        string(APPEND ${OUT_VARIABLE} ",\n")
    endif()
    string(APPEND ${OUT_VARIABLE} "${INDENT}\"${DIR}\": \"${PARENT_DIR}\"")

    # Call for subdirectories recursively
    get_property(SUBDIRS DIRECTORY ${DIR} PROPERTY SUBDIRECTORIES)
    foreach (SUBDIR ${SUBDIRS})
        json_append_lists_files(${OUT_VARIABLE} ${DIR} ${SUBDIR} ${INDENT})
    endforeach ()

    # Add trailing newline
    if ("${PARENT_DIR}d" STREQUAL "d")
        string(APPEND ${OUT_VARIABLE} "\n")
    endif()

    # Propagate to outer scope
    set(${OUT_VARIABLE} ${${OUT_VARIABLE}} PARENT_SCOPE)
endfunction()

function(json_append_globals OUT_VARIABLE INDENT INDENT_INCREMENT)
    set(INNER_INDENT "${INDENT}${INDENT_INCREMENT}")
    set(INNER_INNER_INDENT "${INDENT}${INDENT_INCREMENT}${INDENT_INCREMENT}")

    json_append_line(${OUT_VARIABLE} "{" ${INDENT})
    json_append_key_value_unquoted(${OUT_VARIABLE} darkMode true ${INNER_INDENT} FALSE)
    json_append_key_value(${OUT_VARIABLE} cmagVersion "${CMAG_VERSION}" ${INNER_INDENT} FALSE)
    json_append_key_value(${OUT_VARIABLE} cmakeVersion "${CMAKE_VERSION}" ${INNER_INDENT} FALSE)
    json_append_key_value(${OUT_VARIABLE} cmakeProjectName "${CMAKE_PROJECT_NAME}" ${INNER_INDENT} FALSE)
    json_append_key_value(${OUT_VARIABLE} cmagProjectName "${CMAG_PROJECT_NAME}" ${INNER_INDENT} FALSE)
    json_append_key_value(${OUT_VARIABLE} sourceDir "${CMAKE_SOURCE_DIR}" ${INNER_INDENT} FALSE)
    json_append_key_value(${OUT_VARIABLE} buildDir "${CMAKE_BINARY_DIR}" ${INNER_INDENT} FALSE)
    json_append_key_value(${OUT_VARIABLE} generator "${CMAKE_GENERATOR}" ${INNER_INDENT} FALSE)
    json_append_key_value(${OUT_VARIABLE} compilerId "${CMAKE_CXX_COMPILER_ID}" ${INNER_INDENT} FALSE)
    json_append_key_value(${OUT_VARIABLE} compilerVersion "${CMAKE_CXX_COMPILER_VERSION}" ${INNER_INDENT} FALSE)
    json_append_key_value(${OUT_VARIABLE} os "${CMAKE_SYSTEM_NAME}" ${INNER_INDENT} FALSE)

    json_append_line(${OUT_VARIABLE} "\"listFiles\": {" ${INNER_INDENT})
    json_append_lists_files(${OUT_VARIABLE} "" ${CMAKE_CURRENT_SOURCE_DIR} ${INNER_INNER_INDENT})
    json_append_line(${OUT_VARIABLE} "}" ${INNER_INDENT})

    json_append_line(${OUT_VARIABLE} "}" ${INDENT})

    set(${OUT_VARIABLE} ${${OUT_VARIABLE}} PARENT_SCOPE)
endfunction()





# -------------------------------------------------------------------- Assembling JSON for .cmag-targets-list file
function(json_append_configs OUT_VARIABLE CONFIGS INDENT INDENT_INCREMENT)
    set(INNER_INDENT "${INDENT}${INDENT_INCREMENT}")

    json_append_line(${OUT_VARIABLE} "[" ${INDENT})

    list(LENGTH CONFIGS LAST_CONFIG_INDEX)
    math(EXPR LAST_CONFIG_INDEX "${LAST_CONFIG_INDEX} - 1")
    set(COUNTER 0)
    foreach(CONFIG IN LISTS CONFIGS)
        if(LAST_CONFIG_INDEX EQUAL COUNTER)
            set(IS_LAST TRUE)
        else()
            set(IS_LAST FALSE)
        endif()
        math(EXPR COUNTER "${COUNTER}+1")

        set(LINE "${CMAG_PROJECT_NAME}_${CONFIG}.cmag-targets")
        json_append_line_comma(${OUT_VARIABLE} "\"${LINE}\"" ${INNER_INDENT} ${IS_LAST})
    endforeach()

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
else()
    # Work out current config. It can be unspecified, in which cas we'll name it as "Default".
    # Note that $<CONFIG> generator expression doesn't work if build type isn't explicitly
    # specified, so we have use this placeholder.
    set(CMAG_CONFIG ${CMAKE_BUILD_TYPE})
    if ("${CMAG_CONFIG}d" STREQUAL "d")
        set(CMAG_CONFIG Default)
    endif()

    set(CMAG_CONFIGS "${CMAG_CONFIG}")
    set(CMAG_CONFIG "${CMAG_CONFIG}")
endif()

# Write configs list
json_append_configs(CONFIGS_JSON "${CMAG_CONFIGS}" "  " "  ")
set(TARGETS_LIST_FILE "${CMAKE_BINARY_DIR}/${CMAG_PROJECT_NAME}.cmag-targets-list")
file(WRITE "${TARGETS_LIST_FILE}" "${CONFIGS_JSON}")

# Write global settings
json_append_globals(GLOBALS_JSON "  " "  ")
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
    set(TARGETS_LIST_DEBUG_FILE "${TARGETS_LIST_FILE}.debug")
    message(STATUS "cmag: writing debug file ${TARGETS_LIST_DEBUG_FILE}")
    file(WRITE "${TARGETS_LIST_DEBUG_FILE}" "${TARGETS_JSON}")
endif()

# -----------------------------CMAG POSTAMBLE END---------------------------------------------
)DELIMETER";
