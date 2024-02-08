# -----------------------------CMAG POSTAMBLE BEGIN-------------------------------------------
set(CMAG_MINIMUM_VERSION 3.12.0) # Required for TARGET_GENEX_EVAL
if (CMAKE_VERSION VERSION_LESS CMAG_MINIMUM_VERSION)
    message(FATAL_ERROR "cmag requires minimum CMake version ${CMAG_MINIMUM_VERSION}. Current version is ${CMAKE_VERSION}")
endif()





# -------------------------------------------------------------------- Utilities for JSON assembling
macro(json_append_line OUT_VARIABLE LINE INDENT)
    string(APPEND ${OUT_VARIABLE} "${INDENT}${LINE}\n")
endmacro()

function(json_append_key_value OUT_VARIABLE KEY VALUE INDENT)
    json_append_key_value_unquoted(${OUT_VARIABLE} ${KEY} "\"${VALUE}\"" ${INDENT})

    set(${OUT_VARIABLE} ${${OUT_VARIABLE}} PARENT_SCOPE)
endfunction()

function(json_append_key_value_raw_string OUT_VARIABLE KEY VALUE INDENT)
    # This is wrong on so many levels... Not reading this comment is advised.
    #
    # We dump target properties with CMake generator expressions and embed them in json. These expressions,
    # however, can contain many different characters, including quotes, which are enclosing characters for
    # strings in json. This means we would have to backslash-escape them. We cannot do this in configuration
    # phase. It can be done in generation phase with LIST:TRANSFORM genex, but it introduces a dependency on
    # CMake 3.27.
    #
    # The solution is to enhance nlohmann json library with a custom functionality called "raw-strings". They
    # start and end with a triple apostrophe, like so:
    #    {
    #        "normal_string" : '''our quite "fancy" new string'''
    #    }
    #
    # This allows us to generate json and not escape the quotes. Of course, this will break when some project
    # uses triple apostrophe in one of its property values. Json will probably be misformed.
    json_append_key_value_unquoted(${OUT_VARIABLE} ${KEY} "'''${VALUE}'''" ${INDENT})

    set(${OUT_VARIABLE} ${${OUT_VARIABLE}} PARENT_SCOPE)
endfunction()

function(json_append_key_value_unquoted OUT_VARIABLE KEY VALUE INDENT)
    set(LINE "\"${KEY}\": ${VALUE},")
    json_append_line(${OUT_VARIABLE} ${LINE} ${INDENT})

    set(${OUT_VARIABLE} ${${OUT_VARIABLE}} PARENT_SCOPE)
endfunction()

function(json_append_global_property OUT_VARIABLE KEY PROPERTY_NAME INDENT)
    get_property(VALUE GLOBAL PROPERTY "${PROPERTY_NAME}")
    json_append_key_value(${OUT_VARIABLE} ${KEY} "${VALUE}" ${INDENT})

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
macro(json_append_target_property OUT_VARIABLE TGT NAME PROPERTY INDENT GENEX_EVAL IS_BOOL)
    set(VALUE "$<TARGET_PROPERTY:${TGT},${PROPERTY}>")
    if(${GENEX_EVAL})
        set(VALUE "$<TARGET_GENEX_EVAL:${TGT},${VALUE}>")
    endif()

    if (${IS_BOOL})
        set(VALUE "$<IF:$<BOOL:${VALUE}>,true,false>")
        json_append_key_value_unquoted(${OUT_VARIABLE} "${NAME}" "${VALUE}" ${INDENT})
    else()
        json_append_key_value_raw_string(${OUT_VARIABLE} "${NAME}" "${VALUE}" ${INDENT})
    endif()

endmacro()

function (is_property_allowed_on_target OUT_BOOL TARGET_TYPE PROPERTY_NAME)
    if(CMAKE_VERSION VERSION_LESS "3.19" AND TARGET_TYPE STREQUAL "INTERFACE_LIBRARY" AND NOT PROPERTY_NAME MATCHES "^INTERFACE_")
        set(${OUT_BOOL} FALSE PARENT_SCOPE)
    else()
        set(${OUT_BOOL} TRUE PARENT_SCOPE)
    endif()
endfunction()

function (json_append_target_properties OUT_VARIABLE TGT INDENT INDENT_INCREMENT)
    set(INNER_INDENT "${INDENT}${INDENT_INCREMENT}")
    set(INNER_INNER_INDENT "${INDENT}${INDENT_INCREMENT}${INDENT_INCREMENT}")

    get_target_property(TARGET_TYPE ${TGT} TYPE)

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
        is_property_allowed_on_target(IS_PROPERTY_ALLOWED ${TARGET_TYPE} ${PROP})
        if (NOT ${IS_PROPERTY_ALLOWED})
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
        is_property_allowed_on_target(IS_PROPERTY_ALLOWED ${TARGET_TYPE} ${PROP})
        if (NOT ${IS_PROPERTY_ALLOWED})
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
        is_property_allowed_on_target(IS_PROPERTY_ALLOWED ${TARGET_TYPE} ${PROP})
        if (NOT ${IS_PROPERTY_ALLOWED})
            continue()
        endif()

        json_append_target_property(${OUT_VARIABLE} ${TGT} ${PROP} ${PROP} ${INNER_INDENT} TRUE FALSE)
    endforeach()
    json_append_object_end(${OUT_VARIABLE} ${INDENT})

    set(${OUT_VARIABLE} ${${OUT_VARIABLE}} PARENT_SCOPE)
endfunction()

function (json_append_null_target_properties OUT_VARIABLE INDENT INDENT_INCREMENT)
    json_append_line(${OUT_VARIABLE} "\"non_genexable\": {}," ${INDENT})
    json_append_line(${OUT_VARIABLE} "\"genexable\": {}," ${INDENT})
    json_append_line(${OUT_VARIABLE} "\"genexable_evaled\": {}" ${INDENT})
    set(${OUT_VARIABLE} ${${OUT_VARIABLE}} PARENT_SCOPE)
endfunction()

function(json_append_target OUT_VARIABLE TGT CONFIG INDENT INDENT_INCREMENT)
    set(INNER_INDENT "${INDENT}${INDENT_INCREMENT}")
    set(INNER_INNER_INDENT "${INDENT}${INDENT_INCREMENT}${INDENT_INCREMENT}")
    set(INNER_INNER_INNER_INDENT "${INDENT}${INDENT_INCREMENT}${INDENT_INCREMENT}${INDENT_INCREMENT}")
    set(INNER_INNER_INNER_INNER_INDENT "${INDENT}${INDENT_INCREMENT}${INDENT_INCREMENT}${INDENT_INCREMENT}${INDENT_INCREMENT}")

    get_property(listDir GLOBAL PROPERTY CMAG_LIST_DIR_${TGT})

    json_append_object_begin(${OUT_VARIABLE} "${TGT}" ${INDENT})
    if (TARGET ${TGT})
        json_append_target_property(${OUT_VARIABLE} ${TGT} type TYPE ${INNER_INDENT} FALSE FALSE)
        json_append_key_value(${OUT_VARIABLE} listDir "${listDir}" ${INNER_INDENT})
        json_append_target_property(${OUT_VARIABLE} ${TGT} isImported IMPORTED ${INNER_INDENT} FALSE TRUE)

        json_append_object_begin(${OUT_VARIABLE} "configs" ${INNER_INDENT})
        json_append_object_begin(${OUT_VARIABLE} "${CONFIG}" ${INNER_INNER_INDENT})
        json_append_target_properties(${OUT_VARIABLE} ${TGT} ${INNER_INNER_INNER_INNER_INDENT} ${INDENT_INCREMENT})
        json_append_object_end(${OUT_VARIABLE} ${INNER_INNER_INDENT})
        json_append_object_end(${OUT_VARIABLE} ${INNER_INDENT})
    else()
        # Some imported targets may be not visible. This happens when find_package is not used with GLOBAL option. In
        # that case trying to get their properties results in an error. So we hardcode some values to simplify the parser.
        json_append_key_value(${OUT_VARIABLE} type "UNKNOWN" ${INNER_INDENT})
        json_append_key_value(${OUT_VARIABLE} listDir "${listDir}" ${INNER_INDENT})
        json_append_key_value_unquoted(${OUT_VARIABLE} isImported true ${INNER_INDENT})

        json_append_object_begin(${OUT_VARIABLE} "configs" ${INNER_INDENT})
        json_append_object_begin(${OUT_VARIABLE} "${CONFIG}" ${INNER_INNER_INDENT})
        json_append_null_target_properties(${OUT_VARIABLE} ${INNER_INNER_INNER_INNER_INDENT} ${INDENT_INCREMENT})
        json_append_object_end(${OUT_VARIABLE} ${INNER_INNER_INDENT})
        json_append_object_end(${OUT_VARIABLE} ${INNER_INDENT})
    endif()
    json_append_key_value_unquoted(${OUT_VARIABLE} aliases "[]" ${INNER_INDENT})
    json_append_object_end(${OUT_VARIABLE} ${INDENT})

    set(${OUT_VARIABLE} ${${OUT_VARIABLE}} PARENT_SCOPE)
endfunction()

function (get_all_targets OUT_VARIABLE DIR)
    # Call for this directory
    get_property(BUILDSYSTEM_TARGETS DIRECTORY ${DIR} PROPERTY BUILDSYSTEM_TARGETS)
    get_property(IMPORTED_TARGETS DIRECTORY ${DIR} PROPERTY IMPORTED_TARGETS)
    set(TARGETS ${BUILDSYSTEM_TARGETS} ${IMPORTED_TARGETS})

    # Store list dir for this target in a global property. We cannot use target property, because some targets may not
    # be visible in this postamble.
    foreach(TARGET ${TARGETS})
        set_property(GLOBAL PROPERTY CMAG_LIST_DIR_${TARGET} ${DIR})
    endforeach ()

    # Call for subdirectories recursively
    get_property(SUBDIRS DIRECTORY ${DIR} PROPERTY SUBDIRECTORIES)
    foreach (SUBDIR ${SUBDIRS})
        get_all_targets(${OUT_VARIABLE} ${SUBDIR})
    endforeach ()

    # Propagate to outer scope
    set(${OUT_VARIABLE} ${${OUT_VARIABLE}} ${TARGETS} PARENT_SCOPE)
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

function (json_append_globals_browser OUT_VARIABLE SELECTED_CONFIG INDENT INDENT_INCREMENT)
    json_append_key_value_unquoted(${OUT_VARIABLE} needsLayout true ${INDENT})
    json_append_key_value_unquoted(${OUT_VARIABLE} autoSaveEnabled true ${INDENT})
    json_append_key_value_unquoted(${OUT_VARIABLE} cameraX 0 ${INDENT})
    json_append_key_value_unquoted(${OUT_VARIABLE} cameraY 0 ${INDENT})
    json_append_key_value_unquoted(${OUT_VARIABLE} cameraScale 0 ${INDENT})
    json_append_key_value_unquoted(${OUT_VARIABLE} displayedDependencyType 5 ${INDENT}) # TODO quite hardcoded and magical. How to make it cleaner?
    json_append_key_value_unquoted(${OUT_VARIABLE} selectedTabIndex 1 ${INDENT})
    json_append_key_value(${OUT_VARIABLE} selectedTargetName "" ${INDENT})

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
    json_append_global_property(${OUT_VARIABLE} useFolders USE_FOLDERS ${INNER_INDENT})

    json_append_object_begin(${OUT_VARIABLE} "browser" ${INNER_INDENT})
    json_append_globals_browser(${OUT_VARIABLE} ${SELECTED_CONFIG} ${INNER_INNER_INDENT} ${INDENT_INCREMENT})
    json_append_object_end(${OUT_VARIABLE} ${INNER_INDENT})

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





# -------------------------------------------------------------------- Assembling JSON for .cmag-aliases file
function(json_append_aliases OUT_VARIABLE TARGETS INDENT INDENT_INCREMENT)
    set(INNER_INDENT "${INDENT}${INDENT_INCREMENT}")

    json_append_line(${OUT_VARIABLE} "{" ${INDENT})
    foreach (ALIAS_TARGET ${CMAG_ALIASED_TARGETS})
        if (TARGET ${ALIAS_TARGET})
            get_target_property(ACTUAL_TARGET ${ALIAS_TARGET} ALIASED_TARGET)
            json_append_key_value(${OUT_VARIABLE} "${ALIAS_TARGET}" "${ACTUAL_TARGET}" ${INNER_INDENT})
        endif()
    endforeach()
    json_strip_trailing_comma()
    json_append_line(${OUT_VARIABLE} "}" ${INDENT})

    set(${OUT_VARIABLE} ${${OUT_VARIABLE}} PARENT_SCOPE)
endfunction()





# -------------------------------------------------------------------- Main code
function(cmag_postamble_main)
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

    # Write project name file
    set(PROJECT_NAME_FILE "${CMAKE_BINARY_DIR}/.cmag-project-name")
    if (CMAG_JSON_DEBUG)
        message(STATUS "cmag: generating file ${PROJECT_NAME_FILE}")
    endif()
    file(WRITE "${PROJECT_NAME_FILE}" "${CMAG_PROJECT_NAME}")

    # Write configs list
    set(TARGETS_LIST_FILE "${CMAKE_BINARY_DIR}/${CMAG_PROJECT_NAME}.cmag-targets-list")
    if (CMAG_JSON_DEBUG)
        message(STATUS "cmag: generating file ${TARGETS_LIST_FILE}")
    endif()
    json_append_configs(CONFIGS_JSON "${CMAG_CONFIGS}" "  " "  ")
    file(WRITE "${TARGETS_LIST_FILE}" "${CONFIGS_JSON}")

    # Write global settings
    set(GLOBALS_FILE "${CMAKE_BINARY_DIR}/${CMAG_PROJECT_NAME}.cmag-globals")
    if (CMAG_JSON_DEBUG)
        message(STATUS "cmag: generating file ${GLOBALS_FILE}")
    endif()
    json_append_globals(GLOBALS_JSON "${CMAG_CONFIG_DEFAULT}" "  " "  ")
    file(WRITE "${GLOBALS_FILE}" "${GLOBALS_JSON}")

    # Write per-config targets
    set(TARGETS_LIST_FILE "${CMAKE_BINARY_DIR}/${CMAG_PROJECT_NAME}_${CMAG_CONFIG}.cmag-targets")
    if (CMAG_JSON_DEBUG)
        message(STATUS "cmag: generating file ${TARGETS_LIST_FILE}")
    endif()
    json_append_targets(TARGETS_JSON "${CMAG_CONFIG}" "  " "  ")
    file(GENERATE OUTPUT "${TARGETS_LIST_FILE}" CONTENT "${TARGETS_JSON}")

    if (CMAG_JSON_DEBUG)
        set(TARGETS_LIST_DEBUG_FILE "${CMAKE_BINARY_DIR}/${CMAG_PROJECT_NAME}.cmag-targets.debug")
        message(STATUS "cmag: generating file ${TARGETS_LIST_DEBUG_FILE}")
        file(WRITE "${TARGETS_LIST_DEBUG_FILE}" "${TARGETS_JSON}")
    endif()
endfunction()

function(cmag_postamble_aliases)
    set(ALIASES_FILE "${CMAKE_BINARY_DIR}/${CMAG_PROJECT_NAME}.cmag-aliases")
    if (CMAG_JSON_DEBUG)
        message(STATUS "cmag: generating file ${ALIASES_FILE}")
    endif()
    json_append_aliases(ALIASES_JSON "${CMAG_ALIASED_TARGETS}" "  " "  ")
    file(WRITE "${ALIASES_FILE}" "${ALIASES_JSON}")
endfunction()

# Set cmag project name, if not set explicitly
if ("${CMAG_PROJECT_NAME}d" STREQUAL "d")
    set(CMAG_PROJECT_NAME "${CMAKE_PROJECT_NAME}")
endif()

# Execute main function
if ("${CMAG_MAIN_FUNCTION}" STREQUAL "main")
    cmag_postamble_main()
elseif ("${CMAG_MAIN_FUNCTION}" STREQUAL "aliases")
    cmag_postamble_aliases()
else ()
    message(FATAL_ERROR "cmag: invalid main function")
endif ()

# -----------------------------CMAG POSTAMBLE END---------------------------------------------
