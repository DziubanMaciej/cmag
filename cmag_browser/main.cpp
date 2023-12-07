
#include "cmag_browser/components/config_selector.h"
#include "cmag_browser/components/list_dir_tab.h"
#include "cmag_browser/components/summary_tab.h"
#include "cmag_browser/components/target_folder_tab.h"
#include "cmag_browser/components/target_graph_tab.h"
#include "cmag_browser/ui_utils/cmag_browser_theme.h"
#include "cmag_core/parse/cmag_json_parser.h"
#include "cmag_core/utils/error.h"
#include "cmag_core/utils/file_utils.h"

#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#include <cstdio>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/imgui.h>

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
    FATAL_ERROR_IF(window == nullptr, "failed to initialize graphics context");

    glext::GetProcAddressFn getProcAddress = +[](const char *name) { return reinterpret_cast<void *>(::glfwGetProcAddress(name)); };
    FATAL_ERROR_IF(!glext::initialize(getProcAddress), "failed to initialize OpenGL extensions");

    initializeImgui(window, glslVersion);
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;

    CmagBrowserTheme theme = CmagBrowserTheme::createDarkTheme();
    ConfigSelector configSelector{theme, cmagProject};
    TargetGraphTab targetGraphTab{theme, cmagProject, configSelector};
    ListDirTab listFileTab{theme, cmagProject, targetGraphTab};
    TargetFolderTab targetFolderTab{theme, cmagProject, targetGraphTab};
    SummaryTab summaryTab{theme, cmagProject, configSelector};

    theme.setup();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Record ImGui commands to render our window
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
        ImGui::Begin("ContentWindow", nullptr, windowFlags);
        if (ImGui::BeginTabBar("root")) {
            ImGuiTabItemFlags targetGraphTabFlags = 0;
            if (targetGraphTab.fetchForceSelection()) {
                targetGraphTabFlags |= ImGuiTabItemFlags_SetSelected;
            }

            if (ImGui::BeginTabItem("Target graph", nullptr, targetGraphTabFlags)) {
                targetGraphTab.render(io);
                targetGraphTabFlags = 0;
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("List files")) {
                listFileTab.render();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Target folders")) {
                targetFolderTab.render();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Summary")) {
                summaryTab.render();
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();

        ImGui::End();

        // Actual rendering
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
