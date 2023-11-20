#include "target_graph.h"

#include "cmag_browser/gl_extensions.h"
#include "cmag_lib/core/cmag_project.h"
#include "cmag_lib/parse/cmag_json_parser.h"
#include "cmag_lib/utils/error.h"
#include "cmag_lib/utils/file_utils.h"

#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/imgui.h>
#include <stdio.h>

static void glfw_error_callback(int error, const char *description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

GLFWwindow *initializeWindow(bool coreProfile, const char **outGlslVersion) {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        return nullptr;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    const int profileHint = coreProfile ? GLFW_OPENGL_CORE_PROFILE : GLFW_OPENGL_COMPAT_PROFILE;
    glfwWindowHint(GLFW_OPENGL_PROFILE, profileHint);

    GLFWwindow *window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
    if (window == nullptr) {
        return nullptr;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    *outGlslVersion = "#version 130";
    return window;
}

void deinitializeWindow(GLFWwindow *window) {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void initializeImgui(GLFWwindow *window, const char *glslVersion) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    FATAL_ERROR_IF(!ImGui_ImplGlfw_InitForOpenGL(window, true), "failed to initialized Imgui for GLFW");
    FATAL_ERROR_IF(!ImGui_ImplOpenGL3_Init(glslVersion), "failed to initialized Imgui for OpenGL");
}

int main(int argc, char **argv) {
    FATAL_ERROR_IF(argc < 2, "Specify cmag project file.");
    const char *cmagProjectJsonPath = argv[1];
    const auto cmagProjectJson = readFile(cmagProjectJsonPath);
    FATAL_ERROR_IF(!cmagProjectJson.has_value(), "could not read ", cmagProjectJsonPath);
    CmagProject cmagProject = {};
    FATAL_ERROR_IF(CmagJsonParser::parseProject(cmagProjectJson.value(), cmagProject) != ParseResult::Success, "could not parse ", cmagProjectJsonPath);
    const char *glslVersion = {};
    GLFWwindow *window = initializeWindow(true, &glslVersion);
    FATAL_ERROR_IF(window == nullptr, "failed to initialize graphics context")

    glext::GetProcAddressFn getProcAddress = +[](const char *name) { return reinterpret_cast<void *>(::glfwGetProcAddress(name)); };
    FATAL_ERROR_IF(!glext::initialize(getProcAddress), "failed to initialize OpenGL extensions");

    initializeImgui(window, glslVersion);
    ImGuiIO &io = ImGui::GetIO();

    TargetGraph targetGraph{cmagProject.getTargets()};

    // Our state
    bool show_demo_window = false;

    while (!glfwWindowShouldClose(window)) {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window) {
            ImGui::ShowDemoWindow(&show_demo_window);
        }

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
        ImGui::Begin("Hello, world!", nullptr, windowFlags);
        {
            ImGui::BeginGroup();
            {
                ImGui::Checkbox("Demo Window", &show_demo_window);
                ImGui::Button("Dummy button 1");
                ImGui::Button("Dummy button 2");
                ImGui::Button("Dummy button 3");
                ImGui::EndGroup();
            }

            ImGui::SameLine();

            ImVec2 space = ImGui::GetContentRegionAvail();
            if (space.x > 0 && space.y > 0) {
                int targetGraphW = static_cast<int>(space.x);
                int targetGraphH = static_cast<int>(space.y);

                targetGraph.update(io);
                targetGraph.render(targetGraphW, targetGraphH);
                ImGui::Image((void *)(intptr_t)targetGraph.getTexture(), space);
                const ImVec2 pos = ImGui::GetItemRectMin();
                targetGraph.savePosition(static_cast<size_t>(pos.x), static_cast<size_t>(pos.y));
            }
        }
        ImGui::End();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.7f, 0.7f, 0.7f, 0.7f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    deinitializeWindow(window);
    return 0;
}
