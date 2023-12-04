#pragma once

#include <imgui/imgui.h>

class CmagBrowserTheme;

struct Tooltip {
    static bool begin(const CmagBrowserTheme &theme, ImVec2 min, ImVec2 max, const char *tooltip, const char *tooltipHyperlink);
    static void end();

    static bool isRectHovered(ImVec2 min, ImVec2 max);
};
