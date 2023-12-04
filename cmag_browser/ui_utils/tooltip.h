#pragma once

#include <imgui/imgui.h>

class CmagBrowserTheme;

struct Tooltip {
    static void renderTooltip(const CmagBrowserTheme &theme, ImVec2 min, ImVec2 max, const char *tooltip, const char *tooltipHyperlink);
    static void renderTooltip(const CmagBrowserTheme &theme, const char *tooltip, const char *tooltipHyperlink);

    static bool isRectHovered(ImVec2 min, ImVec2 max);
};
