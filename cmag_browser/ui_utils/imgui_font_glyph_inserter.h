#pragma once

#include <imgui/imgui.h>
#include <vector>

class ImguiFontGlyphInserter {
public:
    ImguiFontGlyphInserter(ImFontAtlas *fontAtlas, ImFont *font, int fontSize);
    ~ImguiFontGlyphInserter();

    void insertGlyph(ImWchar codepoint, const unsigned char *imageData, int imageDataSize);

private:
    struct GlyphData {
        int rectId;
        unsigned char *stbData;
        int components;
    };

    void uploadGlyph(GlyphData &glyphData, ImU32 *atlasMemory, int atlasWidth);

    ImFontAtlas *fontAtlas;
    ImFont *font;
    int fontSize;
    std::vector<GlyphData> glyphDatas;
};
