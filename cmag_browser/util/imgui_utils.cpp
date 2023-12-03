#include "imgui_utils.h"

RaiiImguiStyle::~RaiiImguiStyle() {
    if (colorsCount > 0) {
        ImGui::PopStyleColor(colorsCount);
    }
}

void RaiiImguiStyle::color(ImGuiCol idx, const ImColor &color) {
    ImGui::PushStyleColor(idx, color.Value);
    colorsCount++;
}
