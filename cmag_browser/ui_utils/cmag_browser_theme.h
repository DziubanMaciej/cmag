#pragma once

#include "raii_imgui_style.h"

#include <imgui/imgui.h>

class CmagBrowserTheme {
public:
    RaiiImguiStyle setupText() const;
    RaiiImguiStyle setupPropertyName(bool isEmpty, bool isConsistent) const;
    RaiiImguiStyle setupPropertyValue() const;
    RaiiImguiStyle setupPopup() const;
    RaiiImguiStyle setupPopup(float textWrapWidth) const;
    RaiiImguiStyle setupHyperlink() const;

    const static CmagBrowserTheme darkTheme;

    ImColor colorText = {};
    ImColor colorPropertyName = {};
    ImColor colorPropertyNameEmpty = {};
    ImColor colorPropertyNameInconsistent = {};
    ImColor colorPropertyValue = {};
    ImColor colorPopup = {};
    ImColor colorHyperlink = {};
    float maxWidthPopup = {};
};
