#include "tooltip.h"

#include "cmag_browser/ui_utils/cmag_browser_theme.h"
#include "cmag_core/utils/filesystem.h"

bool Tooltip::begin(const CmagBrowserTheme &theme, ImVec2 min, ImVec2 max, const char *tooltip, const char *tooltipHyperlink, bool forceOneLine) {
    if (!isRectHovered(min, max)) {
        return false;
    }

    if (tooltipHyperlink != nullptr && ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        openHyperlink(tooltipHyperlink);
    }

    if (ImGui::BeginTooltip()) {
        if (tooltipHyperlink) {
            auto textStyle = theme.setupPopup(ImGui::CalcTextSize(tooltipHyperlink).x);
            ImGui::Text("%s", tooltip);

            auto hyperlinkStyle = theme.setupHyperlink();
            ImGui::Text("%s", tooltipHyperlink);
        } else {
            auto textStyle = forceOneLine ? theme.setupPopup(0) : theme.setupPopup();
            ImGui::Text("%s", tooltip);
        }

        return true;
    }

    return false;
}

void Tooltip::end() {
    ImGui::EndTooltip();
}

bool Tooltip::isRectHovered(ImVec2 min, ImVec2 max) {
    const ImVec2 mousePos = ImGui::GetMousePos();
    return min.x <= mousePos.x && mousePos.x <= max.x && min.y <= mousePos.y && mousePos.y <= max.y;
}
