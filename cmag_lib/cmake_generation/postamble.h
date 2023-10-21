const char *postamble = R"DELIMETER(
cmake_minimum_required(VERSION 3.8.0)

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

macro(json_append_key_value OUT_VARIABLE KEY VALUE INDENT)
    json_append_line(${OUT_VARIABLE} "\"${KEY}\" = \"${VALUE}\"" ${INDENT})
endmacro()

macro(json_append_target_property OUT_VARIABLE TGT PROPERTY INDENT)
    set(VALUE "$<TARGET_PROPERTY:${TGT},${PROPERTY}>")
    if("${PROPERTY}" STREQUAL "LINK_LIBRARIES")
        set(VALUE "$<GENEX_EVAL:${VALUE}>")
    endif()
    json_append_key_value(${OUT_VARIABLE} "${PROPERTY}" "${VALUE}" ${INDENT})
endmacro()

macro(json_append_target OUT_VARIABLE TGT INDENT INDENT_INCREMENT)
    set(INNER_INDENT "${INDENT}${INDENT_INCREMENT}")
    json_append_line(${OUT_VARIABLE} "{" ${INDENT})
    json_append_key_value(${OUT_VARIABLE} name ${TGT} ${INNER_INDENT})
    json_append_target_property(${OUT_VARIABLE} ${TGT} INCLUDE_DIRECTORIES ${INNER_INDENT})
    json_append_target_property(${OUT_VARIABLE} ${TGT} LINK_LIBRARIES ${INNER_INDENT})
    json_append_target_property(${OUT_VARIABLE} ${TGT} COMPILE_DEFINITIONS ${INNER_INDENT})
    json_append_target_property(${OUT_VARIABLE} ${TGT} MANUALLY_ADDED_DEPENDENCIES ${INNER_INDENT})
    json_append_line(${OUT_VARIABLE} "}" ${INDENT})
endmacro()

get_all_targets(ALL_TARGETS ${CMAKE_CURRENT_SOURCE_DIR})
foreach (TGT ${ALL_TARGETS})
    json_append_target(JSON ${TGT} "  " "  ")
endforeach ()
message("${JSON}")
file(GENERATE OUTPUT output$<CONFIG>.txt CONTENT "${JSON}")

)DELIMETER";
