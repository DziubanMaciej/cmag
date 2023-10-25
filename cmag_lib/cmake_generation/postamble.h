const char *postamble = R"DELIMETER(

# -----------------------------CMAG POSTAMBLE BEGIN-------------------------------------------
cmake_minimum_required(VERSION 3.9.0)

function (get_all_targets OUT_VARIABLE DIR)
    # Call for this directory
    get_property(TARGETS_IN_CURRENT_DIR DIRECTORY ${DIR} PROPERTY BUILDSYSTEM_TARGETS)
    list(APPEND ${OUT_VARIABLE} ${TARGETS_IN_CURRENT_DIR})

    # Call for subdirectories recursively
    get_property(SUBDIRS DIRECTORY ${DIR} PROPERTY SUBDIRECTORIES)
    foreach (SUBDIR ${SUBDIRS})
        get_all_targets(${OUT_VARIABLE} ${SUBDIR})
    endforeach ()

    # Propagate to outer scope
    set(${OUT_VARIABLE} ${${OUT_VARIABLE}} PARENT_SCOPE)
endfunction()

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

macro(json_append_key_value OUT_VARIABLE KEY VALUE INDENT IS_LAST)
    json_append_key_value_unquoted(${OUT_VARIABLE} ${KEY} "\"${VALUE}\"" ${INDENT} ${IS_LAST})
endmacro()

macro(json_append_key_value_unquoted OUT_VARIABLE KEY VALUE INDENT IS_LAST)
    set(LINE "\"${KEY}\": ${VALUE}")
    if(NOT ${IS_LAST})
        set(LINE "${LINE},")
    endif()
    json_append_line(${OUT_VARIABLE} ${LINE} ${INDENT})
endmacro()

macro(json_append_target_property OUT_VARIABLE TGT PROPERTY INDENT IS_LAST)
    json_append_target_named_property(${OUT_VARIABLE} ${TGT} ${PROPERTY} ${PROPERTY} ${INDENT} ${IS_LAST})
endmacro()

macro(json_append_target_named_property OUT_VARIABLE TGT NAME PROPERTY INDENT IS_LAST)
    set(VALUE "$<TARGET_PROPERTY:${TGT},${PROPERTY}>")
    if("${PROPERTY}" STREQUAL "LINK_LIBRARIES")
        set(VALUE "$<GENEX_EVAL:${VALUE}>")
    endif()
    json_append_key_value(${OUT_VARIABLE} "${NAME}" "${VALUE}" ${INDENT} ${IS_LAST})
endmacro()

function(json_append_target OUT_VARIABLE TGT INDENT INDENT_INCREMENT IS_LAST)
    set(INNER_INDENT "${INDENT}${INDENT_INCREMENT}")
    set(INNER_INNER_INDENT "${INDENT}${INDENT_INCREMENT}${INDENT_INCREMENT}")

    json_append_line(${OUT_VARIABLE} "{" ${INDENT})
    json_append_key_value(${OUT_VARIABLE} name ${TGT} ${INNER_INDENT} FALSE)
    json_append_target_named_property(${OUT_VARIABLE} ${TGT} type TYPE ${INNER_INDENT} FALSE)

    json_append_line(${OUT_VARIABLE} "\"properties\": {" ${INNER_INDENT})
    json_append_target_property(${OUT_VARIABLE} ${TGT} INCLUDE_DIRECTORIES ${INNER_INNER_INDENT} FALSE)
    json_append_target_property(${OUT_VARIABLE} ${TGT} LINK_LIBRARIES ${INNER_INNER_INDENT} FALSE)
    json_append_target_property(${OUT_VARIABLE} ${TGT} COMPILE_DEFINITIONS ${INNER_INNER_INDENT} FALSE)
    json_append_target_property(${OUT_VARIABLE} ${TGT} MANUALLY_ADDED_DEPENDENCIES ${INNER_INNER_INDENT} TRUE)
    json_append_line_comma(${OUT_VARIABLE} "}" ${INNER_INDENT} TRUE)

    json_append_line_comma(${OUT_VARIABLE} "}" ${INDENT} ${IS_LAST})
    set(${OUT_VARIABLE} ${${OUT_VARIABLE}} PARENT_SCOPE)
endfunction()

function(json_append_globals OUT_VARIABLE INDENT INDENT_INCREMENT)
    set(INNER_INDENT "${INDENT}${INDENT_INCREMENT}")
    set(INNER_INNER_INDENT "${INDENT}${INDENT_INCREMENT}${INDENT_INCREMENT}")

    json_append_line(${OUT_VARIABLE} "{" ${INDENT})
    json_append_key_value_unquoted(${OUT_VARIABLE} darkMode true ${INNER_INDENT} TRUE)
    json_append_line(${OUT_VARIABLE} "}" ${INDENT})

    set(${OUT_VARIABLE} ${${OUT_VARIABLE}} PARENT_SCOPE)
endfunction()

function(json_append_targets OUT_VARIABLE CONFIG INDENT INDENT_INCREMENT)
    set(INNER_INDENT "${INDENT}${INDENT_INCREMENT}")
    set(INNER_INNER_INDENT "${INDENT}${INDENT_INCREMENT}${INDENT_INCREMENT}")

    json_append_line(${OUT_VARIABLE} "{" ${INDENT})

    json_append_key_value(${OUT_VARIABLE} "config" "${CONFIG}" ${INNER_INDENT} FALSE)
    json_append_line(${OUT_VARIABLE} "\"targets\": [" ${INNER_INDENT})
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

        json_append_target(${OUT_VARIABLE} ${TGT} ${INNER_INNER_INDENT} ${INDENT_INCREMENT} ${IS_LAST})
    endforeach()

    json_append_line(${OUT_VARIABLE} "]" ${INNER_INDENT})
    json_append_line(${OUT_VARIABLE} "}" ${INDENT})

    set(${OUT_VARIABLE} ${${OUT_VARIABLE}} PARENT_SCOPE)
endfunction()

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

        message("A=${COUNTER} B=${LAST_CONFIG_INDEX} C=${IS_LAST}")
        json_append_line_comma(${OUT_VARIABLE} "\"${CONFIG}\"" ${INNER_INDENT} ${IS_LAST})
    endforeach()

    json_append_line(${OUT_VARIABLE} "]" ${INDENT})

    set(${OUT_VARIABLE} ${${OUT_VARIABLE}} PARENT_SCOPE)
endfunction()

# Get project name
if ("${CMAG_PROJECT_NAME}d" STREQUAL "d")
    set(CMAG_PROJECT_NAME ${CMAKE_PROJECT_NAME})
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
file(WRITE ${CMAKE_BINARY_DIR}/${CMAG_PROJECT_NAME}.cmag-configs "${CONFIGS_JSON}")

# Write global settings
json_append_globals(GLOBALS_JSON "  " "  ")
file(WRITE ${CMAKE_BINARY_DIR}/${CMAG_PROJECT_NAME}.cmag-globals "${GLOBALS_JSON}")

# Write per-config targets
json_append_targets(TARGETS_JSON "${CMAG_CONFIG}" "  " "  ")
file(GENERATE OUTPUT ${CMAKE_BINARY_DIR}/${CMAG_PROJECT_NAME}_${CMAG_CONFIG}.cmag-targets CONTENT "${TARGETS_JSON}")

# -----------------------------CMAG POSTAMBLE END---------------------------------------------
)DELIMETER";
