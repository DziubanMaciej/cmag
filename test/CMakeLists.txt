add_subdirectory(unit)
add_subdirectory(os)

add_test(
    NAME RunOnSelf
    COMMAND $<TARGET_FILE:cmag> -p self_run ${CMAKE_COMMAND} -S "${CMAKE_SOURCE_DIR}" -B "${CMAKE_BINARY_DIR}"
)
