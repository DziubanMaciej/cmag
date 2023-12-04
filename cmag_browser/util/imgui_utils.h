#pragma once

#include "cmag_browser/util/movable_primitive.h"

#include <imgui/imgui.h>

class RaiiImguiStyle {
public:
    RaiiImguiStyle() = default;
    ~RaiiImguiStyle();
    RaiiImguiStyle(RaiiImguiStyle &&) noexcept = default;
    RaiiImguiStyle &operator=(RaiiImguiStyle &&) noexcept = default;

    void color(ImGuiCol idx, const ImColor &color);
    void style(ImGuiStyleVar idx, float val);
    void style(ImGuiStyleVar idx, const ImVec2 &val);
    void textWrapWidth(float width);

private:
    MovablePrimitive<int> colorsCount = 0;
    MovablePrimitive<int> stylesCount = 0;
    MovablePrimitive<int> textWrapPosCount = 0;
};