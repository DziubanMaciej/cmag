#include "cmag_browser_theme.h"

static constexpr ImColor ImColorGrey(float value) {
    return {value, value, value, 1.f};
}

const CmagBrowserTheme CmagBrowserTheme::darkTheme{
    ImColorGrey(1.f),                // colorText
    ImColor::HSV(0.14f, 0.6f, 0.6f), // colorPropertyName;
    ImColor(0.4f, 0.4f, 0.4f, 0.4f), // colorPropertyNameEmpty;
    ImColor::HSV(0.9f, 0.6f, 0.6f),  // colorPropertyNameInconsistent;
    ImColorGrey(0.7f),               // colorPropertyValue;
    ImColorGrey(0.8f),               // colorPopup;
    ImColor(48, 73, 96, 255),        // colorHyperlink;
    300.f,                           // maxWidthPopup;
};

RaiiImguiStyle CmagBrowserTheme::setupText() const {
    RaiiImguiStyle style{};
    style.color(ImGuiCol_Text, colorText);
    return style;
}

RaiiImguiStyle CmagBrowserTheme::setupPropertyName(bool isEmpty, bool isConsistent) const {
    ImColor color = colorPropertyName;
    if (!isConsistent) {
        color = colorPropertyNameInconsistent;
    }
    if (isEmpty) {
        color = colorPropertyNameEmpty;
    }

    RaiiImguiStyle style{};
    style.color(ImGuiCol_Text, color);
    return style;
}

RaiiImguiStyle CmagBrowserTheme::setupPropertyValue() const {
    RaiiImguiStyle style{};
    style.color(ImGuiCol_Text, colorPropertyValue);
    return style;
}

RaiiImguiStyle CmagBrowserTheme::setupPopup() const {
    return setupPopup(maxWidthPopup);
}

RaiiImguiStyle CmagBrowserTheme::setupPopup(float textWrapWidth) const {
    RaiiImguiStyle style{};
    style.color(ImGuiCol_Text, colorPopup);
    style.textWrapWidth(textWrapWidth);
    return style;
}

RaiiImguiStyle CmagBrowserTheme::setupHyperlink() const {
    RaiiImguiStyle style{};
    style.color(ImGuiCol_Text, colorHyperlink);
    return style;
}
