#pragma once

#include "raii_imgui_style.h"

#include <imgui/imgui.h>

class CmagBrowserTheme {
public:
    // Called for entire frame
    void setup() const;

    // Called for rendering individual parts of UI
    RaiiImguiStyle setupPropertyTable() const;
    RaiiImguiStyle setupPropertyName(bool isEmpty, bool isConsistent) const;
    RaiiImguiStyle setupPropertyValue() const;
    RaiiImguiStyle setupPopup() const;
    RaiiImguiStyle setupPopup(float textWrapWidth) const;
    RaiiImguiStyle setupHyperlink() const;

    static CmagBrowserTheme createDarkTheme();

    ImGuiStyle style = {};

    ImColor colorPropertyTableBackground0 = {};
    ImColor colorPropertyTableBackground1 = {};
    ImColor colorPropertyName = {};
    ImColor colorPropertyNameEmpty = {};
    ImColor colorPropertyNameInconsistent = {};
    ImColor colorPropertyValue = {};
    ImColor colorPopup = {};
    ImColor colorHyperlink = {};
    float maxWidthPopup = {};
};
