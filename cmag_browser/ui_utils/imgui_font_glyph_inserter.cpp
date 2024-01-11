#include "imgui_font_glyph_inserter.h"

#include "cmag_core/utils/error.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

ImguiFontGlyphInserter::ImguiFontGlyphInserter(ImFontAtlas *fontAtlas, ImFont *font, int fontSize)
    : fontAtlas(fontAtlas),
      font(font),
      fontSize(fontSize) {}

ImguiFontGlyphInserter::~ImguiFontGlyphInserter() {
    unsigned char *atlasMemory = nullptr;
    int atlasWidth = 0;
    fontAtlas->GetTexDataAsRGBA32(&atlasMemory, &atlasWidth, nullptr);

    for (GlyphData &glyphData : glyphDatas) {
        uploadGlyph(glyphData, reinterpret_cast<ImU32 *>(atlasMemory), atlasWidth);
    }
}

void ImguiFontGlyphInserter::insertGlyph(ImWchar codepoint, const unsigned char *imageData, int imageDataSize) {
    GlyphData data = {};

    int stbDataWidth = 0;
    int stbDataHeight = 0;
    int stbDataComponents = 0;
    data.stbData = stbi_load_from_memory(imageData, imageDataSize, &stbDataWidth, &stbDataHeight, &stbDataComponents, 4);
    data.rectId = fontAtlas->AddCustomRectFontGlyph(font, codepoint, fontSize, fontSize, static_cast<float>(fontSize));
    data.components = stbDataComponents;

    FATAL_ERROR_IF(data.rectId < 0, "Invalid rect ID returned for font glyph");
    FATAL_ERROR_IF(data.stbData == nullptr, "Invalid data for font glyph");

    glyphDatas.push_back(data);
}

void ImguiFontGlyphInserter::uploadGlyph(ImguiFontGlyphInserter::GlyphData &glyphData, ImU32 *atlasMemory, int atlasWidth) {
    const ImFontAtlasCustomRect *rect = fontAtlas->GetCustomRectByIndex(glyphData.rectId);
    FATAL_ERROR_IF(rect == nullptr, "Invalid rect ID used for font glyph");

    for (int y = 0; y < rect->Height; y++) {
        for (int x = 0; x < rect->Width; x++) {
            const int dstPixelIndex = (rect->Y + y) * atlasWidth + rect->X + x;
            ImU32 &dstPixel = atlasMemory[dstPixelIndex];

            const int srcPixelIndex = y * rect->Width + x;
            unsigned char *srcPixel = &glyphData.stbData[glyphData.components * srcPixelIndex];

            dstPixel = IM_COL32(srcPixel[0], srcPixel[1], srcPixel[2], srcPixel[3]);
        }
    }
}
