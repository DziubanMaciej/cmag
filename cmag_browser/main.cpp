
#include "cmag_browser/browser_state/cmag_browser_theme.h"
#include "cmag_browser/browser_state/config_selector.h"
#include "cmag_browser/tabs/list_dir_tab.h"
#include "cmag_browser/tabs/summary_tab.h"
#include "cmag_browser/tabs/target_folder_tab.h"
#include "cmag_browser/tabs/target_graph_tab.h"
#include "cmag_browser/ui_utils/imgui_font_glyph_inserter.h"
#include "cmag_core/browser/browser_argument_parser.h"
#include "cmag_core/core/version.h"
#include "cmag_core/parse/cmag_json_parser.h"
#include "cmag_core/utils/error.h"
#include "cmag_core/utils/file_utils.h"

#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#include <cstdio>
#include <generated/cmake_icon.h>
#include <generated/folder_icon.h>
#include <generated/font.h>
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

int main(int argc, const char **argv) {
    // Parse arguments
    BrowserArgumentParser argParser{argc, argv};
    if (argParser.getShowVersion()) {
        LOG_INFO(cmagVersion.toString());
        return 0;
    }
    if (!argParser.isValid()) {
        LOG_ERROR(argParser.getErrorMessage());
        argParser.printHelp();
        return 1;
    }

    // Load cmag project
    const auto cmagProjectJson = readFile(argParser.getProjectFilePath());
    if (!cmagProjectJson.has_value()) {
        LOG_ERROR("could not read project file ", argParser.getProjectFilePath());
        return 1;
    }
    CmagProject cmagProject = {};
    const ParseResult projectParseResult = CmagJsonParser::parseProject(cmagProjectJson.value(), cmagProject);
    if (projectParseResult.status != ParseResultStatus::Success) {
        LOG_ERROR("could not parse project file ", argParser.getProjectFilePath(), ". ", projectParseResult.errorMessage);
        return 1;
    }

    // Init OpenGL
    const char *glslVersion = {};
    GLFWwindow *window = initializeWindow(true, &glslVersion);
    if (window == nullptr) {
        LOG_ERROR("could not initialize OpenGL context");
        return 1;
    }
    glext::GetProcAddressFn getProcAddress = +[](const char *name) { return reinterpret_cast<void *>(::glfwGetProcAddress(name)); };
    if (!glext::initialize(getProcAddress)) {
        LOG_ERROR("could not initialize OpenGL extensions");
        return 1;
    }

    // Init ImGui
    initializeImgui(window, glslVersion);
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;

    // Init ImGui font
    {
        ImFontConfig fontConfig{};
        fontConfig.FontDataOwnedByAtlas = false;
        auto mainFont = io.Fonts->AddFontFromMemoryTTF(const_cast<unsigned char *>(cmagBrowserFont), sizeof(cmagBrowserFont), 64.0f, &fontConfig);
        {
            ImguiFontGlyphInserter inserter{io.Fonts, mainFont, 64};
            inserter.insertGlyph(0x1A4, folderIconPngBytes, sizeof(folderIconPngBytes));
            inserter.insertGlyph(0x1A5, cmakeIconPngBytes, sizeof(cmakeIconPngBytes));
        }
        mainFont->Scale = 18.f / mainFont->FontSize;
    }

    // Init browser components
    CmagBrowserTheme theme = CmagBrowserTheme::createDarkTheme();
    BrowserState browserState{theme, argParser.getProjectFilePath(), cmagProject};

    TargetGraphTab targetGraphTab{browserState, argParser.getShowDebugWidgets()};
    ListDirTab listFileTab{browserState};
    TargetFolderTab targetFolderTab{browserState};
    SummaryTab summaryTab{browserState};
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
            const TabChange::TabSelection selection = browserState.getTabChange().fetch();
            auto createTabItemFlags = [&selection](TabChange::TabSelection tab) { return tab == selection ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None; };

            if (ImGui::BeginTabItem("Target graph", nullptr, createTabItemFlags(TabChange::TargetGraph))) {
                targetGraphTab.render(io);
                browserState.getTabChange().setLastDisplayedTab(TabChange::TargetGraph);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("List files", nullptr, createTabItemFlags(TabChange::ListDir))) {
                listFileTab.render();
                browserState.getTabChange().setLastDisplayedTab(TabChange::ListDir);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Target folders", nullptr, createTabItemFlags(TabChange::TargetFolder))) {
                targetFolderTab.render();
                browserState.getTabChange().setLastDisplayedTab(TabChange::TargetFolder);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Summary", nullptr, createTabItemFlags(TabChange::Summary))) {
                summaryTab.render();
                browserState.getTabChange().setLastDisplayedTab(TabChange::Summary);
                ImGui::EndTabItem();
            }

            browserState.getTabChange().renderPopup();
            browserState.getProjectSaver().tryAutoSave();
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
