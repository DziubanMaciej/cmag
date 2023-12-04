#include "raii_imgui_style.h"

RaiiImguiStyle::~RaiiImguiStyle() {
    if (colorsCount > 0) {
        ImGui::PopStyleColor(colorsCount);
    }
    if (stylesCount > 0) {
        ImGui::PopStyleVar(stylesCount);
    }
    for (int i = 0; i < textWrapPosCount; i++) {
        ImGui::PopTextWrapPos();
    }
}

void RaiiImguiStyle::color(ImGuiCol idx, const ImColor &color) {
    ImGui::PushStyleColor(idx, color.Value);
    colorsCount = colorsCount + 1;
}

void RaiiImguiStyle::style(ImGuiStyleVar idx, float val) {
    ImGui::PushStyleVar(idx, val);
    stylesCount = stylesCount + 1;
}

void RaiiImguiStyle::style(ImGuiStyleVar idx, const ImVec2 &val) {
    ImGui::PushStyleVar(idx, val);
    stylesCount = stylesCount + 1;
}
void RaiiImguiStyle::textWrapWidth(float width) {
    ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + width);
    textWrapPosCount = textWrapPosCount + 1;
}
