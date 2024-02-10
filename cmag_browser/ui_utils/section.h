#pragma once

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
        ImGui::Dummy(ImVec2(0, verticalMargin));
    }

    float indent;
    float verticalMargin;
};
