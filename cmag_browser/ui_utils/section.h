#pragma once

#include "cmag_browser/util/movable_primitive.h"

#include <imgui/imgui.h>

struct Section {
    Section(const char *sectionName, float indent, float verticalMargin)
        : indent(indent),
          verticalMargin(verticalMargin) {
        ImGui::SeparatorText(sectionName);
        if (indent != 0) {
            ImGui::Indent(indent);
        }
    }

    ~Section() {
        if (indent != 0) {
            ImGui::Unindent(indent);
        }
        if (verticalMargin != 0) {
            ImGui::Dummy(ImVec2(0, verticalMargin));
        }
    }

    Section(Section &&) noexcept = default;
    Section &operator=(Section &&) noexcept = default;

    MovablePrimitive<float> indent;
    MovablePrimitive<float> verticalMargin;
};
