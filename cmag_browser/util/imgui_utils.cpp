#include "imgui_utils.h"

RaiiImguiStyle::~RaiiImguiStyle() {
    if (colorsCount > 0) {
        ImGui::PopStyleColor(colorsCount);
    }
    if (stylesCount) {
        ImGui::PopStyleVar(stylesCount);
    }
}

void RaiiImguiStyle::color(ImGuiCol idx, const ImColor &color) {
    ImGui::PushStyleColor(idx, color.Value);
    colorsCount++;
}

void RaiiImguiStyle::style(ImGuiStyleVar idx, float val) {
    ImGui::PushStyleVar(idx, val);
    stylesCount++;
}

void RaiiImguiStyle::style(ImGuiStyleVar idx, const ImVec2 &val) {
    ImGui::PushStyleVar(idx, val);
    stylesCount++;
}
