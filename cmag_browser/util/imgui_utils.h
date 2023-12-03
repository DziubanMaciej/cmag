#pragma once

#include <imgui/imgui.h>

class RaiiImguiStyle {
public:
    ~RaiiImguiStyle();

    void color(ImGuiCol idx, const ImColor &color);

private:
    int colorsCount = 0;
};