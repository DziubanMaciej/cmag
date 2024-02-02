#include "tooltip.h"

#include "cmag_browser/ui_utils/cmag_browser_theme.h"
#include "cmag_core/utils/error.h"
#include "cmag_core/utils/filesystem.h"

TooltipBuilder::TooltipBuilder(const CmagBrowserTheme &theme)
    : theme(theme) {}

TooltipBuilder &TooltipBuilder::setHoverRect(ImVec2 rectMin, ImVec2 rectMax) {
    hover.type = Hover::Type::Rect;
    hover.rectMin = rectMin;
    hover.rectMax = rectMax;
    return *this;
}

TooltipBuilder &TooltipBuilder::setHoverLastItem() {
    hover.type = Hover::Type::LastItem;
    return *this;
}

TooltipBuilder &TooltipBuilder::setHoverAlways() {
    hover.type = Hover::Type::Always;
    return *this;
}

TooltipBuilder &TooltipBuilder::addHyperlink(const char *newHyperlink) {
    hyperlink.hyperlink = newHyperlink;
    return *this;
}

TooltipBuilder &TooltipBuilder::addText(const char *text) {
    FATAL_ERROR_IF(texts.textsCount >= texts.maxTexts, "Too many texts pushed to TooltipBuilder");

    texts.texts[texts.textsCount].text = text;
    texts.texts[texts.textsCount].forceOneLine = false;
    texts.textsCount++;

    return *this;
}

TooltipBuilder &TooltipBuilder::addTextOneLine(const char *text) {
    FATAL_ERROR_IF(texts.textsCount >= texts.maxTexts, "Too many texts pushed to TooltipBuilder");

    texts.texts[texts.textsCount].text = text;
    texts.texts[texts.textsCount].forceOneLine = true;
    texts.textsCount++;

    return *this;
}

void TooltipBuilder::execute() {
    static const auto emptyBody = []() {};
    execute(emptyBody);
}

bool TooltipBuilder::Hover::isTooltipVisible() const {
    switch (type) {
    case Hover::Type::Rect: {
        const ImVec2 mousePos = ImGui::GetMousePos();
        return rectMin.x <= mousePos.x && mousePos.x <= rectMax.x && rectMin.y <= mousePos.y && mousePos.y <= rectMax.y;
    }
    case Hover::Type::LastItem:
        return ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled);
    case Hover::Type::Always:
        return true;
    default:
        FATAL_ERROR("Unknown Hover::Type");
    }
}

void TooltipBuilder::Texts::render(const CmagBrowserTheme &theme) const {
    for (int i = 0; i < textsCount; i++) {
        if (texts[i].text[0] == '\0') {
            continue;
        }

        RaiiImguiStyle oneLineStyle{};
        if (!texts[i].forceOneLine) {
            oneLineStyle.textWrapWidth(theme.maxWidthPopup);
        }

        ImGui::Text("%s", texts[i].text);
    }
}
void TooltipBuilder::Hyperlink::render(const CmagBrowserTheme &theme) const {
    if (hyperlink) {
        auto hyperlinkStyle = theme.setupHyperlink();
        ImGui::Text("%s", hyperlink);
    }

    if (hyperlink != nullptr && ImGui::GetIO().MouseClicked[ImGuiMouseButton_Left]) {
        openHyperlink(hyperlink);
    }
}
