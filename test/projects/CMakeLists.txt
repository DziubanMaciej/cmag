add_test(NAME AcceptanceTests COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/main.py ${CMAKE_BINARY_DIR}/test_files $<TARGET_FILE:cmag>)

add_subdirectories()
