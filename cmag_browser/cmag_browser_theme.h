#pragma once

#include "cmag_browser/util/imgui_utils.h"

#include <imgui/imgui.h>

class CmagBrowserTheme {
public:
    RaiiImguiStyle setupText() const;
    RaiiImguiStyle setupPropertyName(bool isEmpty, bool isConsistent) const;
    RaiiImguiStyle setupPropertyValue() const;
    RaiiImguiStyle setupPopup() const;

    const static CmagBrowserTheme darkTheme;

    ImColor colorText = {};
    ImColor colorPropertyName = {};
    ImColor colorPropertyNameEmpty = {};
    ImColor colorPropertyNameInconsistent = {};
    ImColor colorPropertyValue = {};
    ImColor colorPopup = {};
    float maxWidthPopup = {};
};
