add_library(Imgui STATIC
    imgui/imgui.h
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_glfw.h
    imgui/backends/imgui_impl_opengl3.cpp
    imgui/backends/imgui_impl_opengl3.h
    imgui/imconfig.h
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_internal.h
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imstb_rectpack.h
    imgui/imstb_textedit.h
    imgui/imstb_truetype.h
)
target_include_directories(Imgui INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(Imgui PUBLIC imgui)
target_link_libraries(Imgui PUBLIC glfw OpenGL::GL)
target_compile_definitions(Imgui PUBLIC IMGUI_DISABLE_OBSOLETE_FUNCTIONS)
set_target_properties(Imgui PROPERTIES FOLDER ThirdParty)
