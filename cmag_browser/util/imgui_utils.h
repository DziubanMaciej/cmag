#pragma once

#include <imgui/imgui.h>

class RaiiImguiStyle {
public:
    ~RaiiImguiStyle();

    void color(ImGuiCol idx, const ImColor &color);
    void style(ImGuiStyleVar idx, float val);
    void style(ImGuiStyleVar idx, const ImVec2 &val);

private:
    int colorsCount = 0;
    int stylesCount = 0;
};