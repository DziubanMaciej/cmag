#pragma once

#include <imgui/imgui.h>

class CmagBrowserTheme;

class TooltipBuilder {
public:
    explicit TooltipBuilder(const CmagBrowserTheme &theme);

    TooltipBuilder &setHoverRect(ImVec2 rectMin, ImVec2 rectMax);
    TooltipBuilder &setHoverLastItem();
    TooltipBuilder &setHoverAlways();

    TooltipBuilder &addText(const char *text);
    TooltipBuilder &addTextOneLine(const char *text);

    TooltipBuilder &addHyperlink(const char *newHyperlink);

    template <class BodyFunc>
    void execute(BodyFunc body);
    void execute();

private:
    const CmagBrowserTheme &theme;

    struct Hover {
        enum class Type {
            Always,
            LastItem,
            Rect,
        };
        Type type = Type::Always;
        ImVec2 rectMin = {};
        ImVec2 rectMax = {};

        bool isTooltipVisible() const;
    } hover = {};

    struct Texts {
        struct Text {
            const char *text = nullptr;
            bool forceOneLine = false;
        };

        constexpr static inline int maxTexts = 4;
        Text texts[maxTexts] = {};
        int textsCount = 0;

        void render(const CmagBrowserTheme &theme) const;
    } texts = {};

    struct Hyperlink {
        const char *hyperlink = nullptr;

        void render(const CmagBrowserTheme &theme) const;
    } hyperlink = {};
};

template <class BodyFunc>
inline void TooltipBuilder::execute(BodyFunc body) {
    if (!hover.isTooltipVisible()) {
        return;
    }

    if (ImGui::BeginTooltip()) {
        texts.render(theme);
        body();
        hyperlink.render(theme);

        ImGui::EndTooltip();
    }
}
