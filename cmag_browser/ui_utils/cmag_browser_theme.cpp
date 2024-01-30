#include "cmag_browser_theme.h"

static constexpr ImColor ImColorGrey(float value) {
    return {value, value, value, 1.f};
}

const static ImColor debugColor[] = {
    ImColor(255, 0, 0, 255),
    ImColor(0, 255, 0, 255),
    ImColor(0, 0, 255, 255),
    ImColor(255, 255, 0, 255),
    ImColor(255, 0, 255, 255),
    ImColor(0, 255, 255, 255),
};

CmagBrowserTheme CmagBrowserTheme::createDarkTheme() {
    // Define our color palette
    ImColor eggplant{0x55, 0x3a, 0x41};
    ImColor roseTaupe{0x86, 0x5B, 0x66};
    ImColor raisinBlack{0x26, 0x1F, 0x1E};
    ImColor vistaBlue{0x8E, 0xA4, 0xD2};
    ImColor vistaBlueLighter{0x7C, 0x95, 0xCB};
    ImColor indigoDye{0x03, 0x49, 0x63};

    ImColor grayBackground = ImColorGrey(0.1f);
    ImColor grayBackgroundLighter = ImColorGrey(0.11f);
    ImColor grayBackgroundLighterLighter = ImColorGrey(0.118f);

    ImColor &colorWidgetMark = vistaBlue;
    ImColor &colorWidgetMarkClicked = vistaBlueLighter;

    // Create basic theme
    CmagBrowserTheme theme = {
        ImGuiStyle{},                          // style
        grayBackgroundLighter,                 // colorPropertyTableBackground0;
        grayBackgroundLighterLighter,          // colorPropertyTableBackground1;
        vistaBlue,                             // colorPropertyName;
        ImColor(0.4f, 0.4f, 0.4f, 0.4f),       // colorPropertyNameEmpty;
        roseTaupe,                             // colorPropertyNameInconsistent;
        ImColorGrey(0.7f),                     // colorPropertyValue;
        ImColorGrey(0.8f),                     // colorPopup;
        ImColor(48, 73, 96, 255),              // colorHyperlink;
        ImColor(255, 204, 0, 255),             // colorWarning;
        ImColor(grayBackgroundLighterLighter), // colorTargetGraphBackground
        vistaBlue,                             // colorTargetGraphNode
        vistaBlueLighter,                      // colorTargetGraphNodeFocused
        indigoDye,                             // colorTargetGraphNodeSelected
        ImColorGrey(0),                        // colorTargetGraphNodeOutline
        vistaBlue,                             // colorTargetGraphConnection
        vistaBlueLighter,                      // colorTargetGraphConnectionFocused
        indigoDye,                             // colorTargetGraphConnectionSelected
        300.f,                                 // maxWidthPopup;
    };
    ImGui::StyleColorsDark(&theme.style);
    ImGuiStyle &style = theme.style;

    // Define rest of the style
    style.Colors[ImGuiCol_Text] = ImColorGrey(1.f);
    style.Colors[ImGuiCol_WindowBg] = grayBackground;
    style.WindowBorderSize = 0.0f;

    // Frames are input widget such as checkboxes, sliders, combo-boxes
    style.Colors[ImGuiCol_FrameBg] = ImColorGrey(0.2f);
    style.Colors[ImGuiCol_FrameBgActive] = ImColorGrey(0.26f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImColorGrey(0.3f);
    style.FrameRounding = 2.f;

    // Popups
    style.Colors[ImGuiCol_PopupBg] = grayBackground;

    // Checkmarks, slider handles, etc
    style.Colors[ImGuiCol_CheckMark] = colorWidgetMark;
    style.Colors[ImGuiCol_SliderGrab] = colorWidgetMark;
    style.Colors[ImGuiCol_SliderGrabActive] = colorWidgetMarkClicked;
    style.Colors[ImGuiCol_Header] = colorWidgetMark;
    style.Colors[ImGuiCol_HeaderHovered] = colorWidgetMark;
    style.Colors[ImGuiCol_HeaderActive] = colorWidgetMarkClicked;
    style.Colors[ImGuiCol_Button] = colorWidgetMark;
    style.Colors[ImGuiCol_ButtonHovered] = colorWidgetMark;
    style.Colors[ImGuiCol_ButtonActive] = colorWidgetMarkClicked;

    // Tab bars
    style.Colors[ImGuiCol_Tab] = vistaBlue;
    style.Colors[ImGuiCol_TabHovered] = indigoDye; // when tab is both hovered and active, this one is used...
    style.Colors[ImGuiCol_TabActive] = indigoDye;

    return theme;
}

void CmagBrowserTheme::setup() const {
    memcpy(&ImGui::GetStyle(), &style, sizeof(ImGuiStyle));
}

RaiiImguiStyle CmagBrowserTheme::setupPropertyTable() const {
    RaiiImguiStyle raiiStyle{};
    raiiStyle.color(ImGuiCol_TableRowBg, colorPropertyTableBackground0);
    raiiStyle.color(ImGuiCol_TableRowBgAlt, colorPropertyTableBackground1);
    return raiiStyle;
}

RaiiImguiStyle CmagBrowserTheme::setupPropertyName(bool isEmpty, bool isConsistent) const {
    ImColor color = colorPropertyName;
    if (isEmpty) {
        color = colorPropertyNameEmpty;
    }
    if (!isConsistent) {
        color = colorPropertyNameInconsistent;
    }

    RaiiImguiStyle raiiStyle{};
    raiiStyle.color(ImGuiCol_Text, color);
    return raiiStyle;
}

RaiiImguiStyle CmagBrowserTheme::setupPropertyValue() const {
    RaiiImguiStyle raiiStyle{};
    raiiStyle.color(ImGuiCol_Text, colorPropertyValue);
    return raiiStyle;
}

RaiiImguiStyle CmagBrowserTheme::setupPopup() const {
    return setupPopup(maxWidthPopup);
}

RaiiImguiStyle CmagBrowserTheme::setupPopup(float textWrapWidth) const {
    RaiiImguiStyle raiiStyle{};
    raiiStyle.color(ImGuiCol_Text, colorPopup);
    if (textWrapWidth > 0) {
        raiiStyle.textWrapWidth(textWrapWidth);
    }
    return raiiStyle;
}

RaiiImguiStyle CmagBrowserTheme::setupHyperlink() const {
    RaiiImguiStyle raiiStyle{};
    raiiStyle.color(ImGuiCol_Text, colorHyperlink);
    return raiiStyle;
}
