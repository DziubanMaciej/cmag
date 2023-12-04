#include "tooltip.h"

#include "cmag_browser/ui_utils/cmag_browser_theme.h"

#include <cstdio>
#include <cstdlib>

// TODO refactor into utils
void openHyperlink(const char *path) {
#ifdef _WIN32
    // Note: executable path must use backslashes!
    ::ShellExecuteA(NULL, "open", path, NULL, NULL, SW_SHOWDEFAULT);
#else
#if __APPLE__
    const char *open_executable = "open";
#else
    const char *open_executable = "xdg-open";
#endif
    char command[256];
    snprintf(command, 256, "%s \"%s\"", open_executable, path);
    system(command);
#endif
}

void Tooltip::renderTooltip(const CmagBrowserTheme &theme, ImVec2 min, ImVec2 max, const char *tooltip, const char *tooltipHyperlink) {
    if (isRectHovered(min, max)) {
        renderTooltip(theme, tooltip, tooltipHyperlink);
    }
}

void Tooltip::renderTooltip(const CmagBrowserTheme &theme, const char *tooltip, const char *tooltipHyperlink) {
    if (tooltipHyperlink != nullptr && ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        openHyperlink(tooltipHyperlink);
    }

    if (ImGui::BeginTooltip()) {
        {
            auto textStyle = theme.setupPopup();
            ImGui::Text("%s", tooltip);
        }
        if (tooltipHyperlink) {
            auto hyperlinkStyle = theme.setupHyperlink();
            ImGui::Text("%s", tooltipHyperlink);
        }
        ImGui::EndTooltip();
    }
}

bool Tooltip::isRectHovered(ImVec2 min, ImVec2 max) {
    const ImVec2 mousePos = ImGui::GetMousePos();
    return min.x <= mousePos.x && mousePos.x <= max.x && min.y <= mousePos.y && mousePos.y <= max.y;
}
