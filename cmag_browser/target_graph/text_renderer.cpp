#include "text_renderer.h"

#include "cmag_browser/util/gl_extensions.h"
#include "cmag_browser/util/gl_helpers.h"
#include "cmag_core/utils/math_utils.h"

#include <algorithm>
#include <cstdint>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>

constexpr GLuint verticesInQuad = 6;
constexpr GLuint componentsInVertex = 4; // x,y,u,v

TextRenderer::TextRenderer(float nodeScale, float textScale) {
    allocateProgram();
    setScalesAndInvalidate(nodeScale, textScale);
}

TextRenderer::~TextRenderer() {
    if (gl.program) {
        glDeleteProgram(gl.program);
    }
}

void TextRenderer::setScalesAndInvalidate(float nodeScale, float textScale) {
    FATAL_ERROR_IF(nodeScale == 0, "Zero nodeScale is not valid");
    FATAL_ERROR_IF(textScale == 0, "Zero textScale is not valid");

    const float newRatio = nodeScale / textScale;
    if (newRatio == nodeToTextScaleRatio) {
        return;
    }

    nodeToTextScaleRatio = newRatio;
    strings.clear();
}

void TextRenderer::render(glm::mat4 transform, float depthValue, std::string_view text, ImFont *font) {
    const PerStringData &data = getStringData(text, font);

    SAFE_GL(glBindVertexArray(data.gl.vao));
    SAFE_GL(glEnableVertexAttribArray(0));
    SAFE_GL(glEnableVertexAttribArray(1));
    SAFE_GL(glBindTexture(GL_TEXTURE_2D, getTextureId(font)));
    SAFE_GL(glUseProgram(gl.program));
    SAFE_GL(glEnable(GL_BLEND));
    SAFE_GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    SAFE_GL(glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA));

    SAFE_GL(glUniform2f(gl.programUniform.miscValues, depthValue, data.xOffset));
    SAFE_GL(glUniformMatrix4fv(gl.programUniform.transform, 1, GL_FALSE, glm::value_ptr(transform)));
    SAFE_GL(glDrawArrays(GL_TRIANGLES, 0, data.vertexCount));

    SAFE_GL(glDisable(GL_BLEND));
    SAFE_GL(glBindVertexArray(0));
    SAFE_GL(glDisableVertexAttribArray(0));
    SAFE_GL(glDisableVertexAttribArray(1));
    SAFE_GL(glBindTexture(GL_TEXTURE_2D, 0));
    SAFE_GL(glUseProgram(0));
}

TextRenderer::PerStringData &TextRenderer::getStringData(std::string_view text, ImFont *font) {
    auto it = std::find_if(strings.begin(), strings.end(), [text](const PerStringData &data) {
        return data.string == text;
    });
    if (it != strings.end()) {
        return *it;
    }

    strings.emplace_back(font, text, nodeToTextScaleRatio);
    return strings.back();
}

GLuint TextRenderer::getTextureId(ImFont *font) {
    FATAL_ERROR_IF(font == nullptr, "ImGui font not loaded yet.");

    GLuint texture = static_cast<GLuint>(reinterpret_cast<uintptr_t>(font->ContainerAtlas->TexID));
    FATAL_ERROR_IF(texture == 0, "Invalid texture id");
    return texture;
}

void TextRenderer::allocateProgram() {
    const char *vertexShaderSource = R"(
    #version 330 core
    #extension GL_ARB_separate_shader_objects : enable

    uniform vec2 miscValues;
    uniform mat4 transform;
    in vec2 inPos;
    in vec2 inTexCoord;

    layout(location = 0) out vec2 outTexCoord;
    void main() {
        float depthValue = miscValues.x;
        float xOffset = miscValues.y;

        gl_Position = vec4(inPos.x + xOffset, inPos.y, depthValue, 1);
        gl_Position = transform * gl_Position;

        outTexCoord = inTexCoord;
    }
)";
    const char *fragmentShaderSource = R"(
    #version 330 core
    #extension GL_ARB_separate_shader_objects : enable

    uniform sampler2D fontTexture;
    layout(location = 0) in vec2 inTexCoord;
    out vec4 outColor;
    void main()
    {
        outColor = texture(fontTexture, inTexCoord.st);
    }

)";
    gl.program = createProgram(vertexShaderSource, fragmentShaderSource);
    gl.programUniform.miscValues = getUniformLocation(gl.program, "miscValues");
    gl.programUniform.transform = getUniformLocation(gl.program, "transform");
}

TextRenderer::PerStringData::PerStringData(ImFont *font, std::string_view text, float nodeToTextScaleRatio) {
    string = std::string{text};

    const std::vector<float> vertexData = prepareVertexData(font, text, nodeToTextScaleRatio, &vertexCount, &xOffset);
    GLint attribSizes[] = {2, 2};
    createVertexBuffer(&gl.vao, &gl.vbo, vertexData.data(), vertexData.size() * sizeof(float), attribSizes, 2u);
}

TextRenderer::PerStringData::~PerStringData() {
    if (gl.vbo) {
        glDeleteBuffers(1, &gl.vbo);
    }
    if (gl.vao) {
        glDeleteVertexArrays(1, &gl.vao);
    }
}

std::vector<float> TextRenderer::PerStringData::prepareVertexData(ImFont *font, std::string_view text, float nodeToTextScaleRatio, GLuint *outVertexCount, float *outXOffset) {

    std::vector<float> result = {};
    result.reserve(text.length() * verticesInQuad * componentsInVertex);

    // Calculate min and max height of our font. To get an idea in what space it is defined.
    float minHeight = std::numeric_limits<float>::max();
    float maxHeight = std::numeric_limits<float>::min();
    for (unsigned char i = 'a'; i < 'z'; i++) {
        const ImFontGlyph *glyph = font->FindGlyphNoFallback(i);
        if (glyph->Y1 > maxHeight) {
            maxHeight = glyph->Y1;
        }
        if (glyph->Y0 < minHeight) {
            minHeight = glyph->Y0;
        }
    }

    // Our glyphs are in some custom ImGui space. We need them in predictable model space
    // which we define as <-1, 1>, so we interpolate them before adding to the vertex buffer.
    auto transformToModelSpaceX = [minHeight, maxHeight](float value, bool isDiff = false) {
        const float srcMin = 0;
        const float srcMax = (maxHeight - minHeight) / 2;
        if (isDiff) {
            return interpolateDifference(value, srcMin, srcMax, 0.f, 1.f);
        } else {
            return interpolate(value, srcMin, srcMax, 0.f, 1.f);
        }
    };
    auto transformToModelSpaceY = [minHeight, maxHeight](float value, bool isDiff = false) {
        const float srcMin = minHeight;
        const float srcMax = maxHeight;
        if (isDiff) {
            return interpolateDifference(value, srcMin, srcMax, -1.f, 1.f);
        } else {
            return interpolate(value, srcMin, srcMax, -1.f, 1.f);
        }
    };

    // Calculate width of ellipsis - three dots that are displayed if the text is too long
    const ImFontGlyph *dotGlyph = font->FindGlyphNoFallback('.');
    const float ellipsisWidth = transformToModelSpaceX(dotGlyph->AdvanceX * 3, true);

    // Calculate width of the text
    const float maxTextWidth = 2 * nodeToTextScaleRatio;
    float textWidth = 0;
    size_t charactersCount = 0;
    bool ellipsisNeeded = false;
    for (char character : text) {
        // Add current character to the text
        charactersCount++;
        textWidth += transformToModelSpaceX(font->FindGlyphNoFallback(character)->AdvanceX, true);

        // Check if we exceeded maximum width. If yes, then we have to start subtracting characters we've added
        // until the text + three dots (ellipsis) fits in the maximum width.
        if (textWidth > maxTextWidth) {
            ellipsisNeeded = true;

            while (textWidth + ellipsisWidth > maxTextWidth && charactersCount > 0) {
                const ImFontGlyph *glyphToRemove = font->FindGlyphNoFallback(text[charactersCount - 1]);
                textWidth -= transformToModelSpaceX(glyphToRemove->AdvanceX, true);
                charactersCount--;
            }

            textWidth += ellipsisWidth;

            break;
        }
    }

    float currentX = 0;

    const auto appendCharacterVertices = [&](char c) {
        const ImFontGlyph *glyph = font->FindGlyphNoFallback(c);

        const float x0 = transformToModelSpaceX(glyph->X0) + currentX;
        const float y0 = transformToModelSpaceY(glyph->Y0);
        const float x1 = transformToModelSpaceX(glyph->X1) + currentX;
        const float y1 = transformToModelSpaceY(glyph->Y1);
        currentX += transformToModelSpaceX(glyph->AdvanceX, true);

#define VERTEX(index_x, index_y)         \
    result.push_back(x##index_x);        \
    result.push_back(y##index_y);        \
    result.push_back(glyph->U##index_x); \
    result.push_back(glyph->V##index_y);
        VERTEX(0, 0);
        VERTEX(1, 1);
        VERTEX(0, 1);

        VERTEX(0, 0);
        VERTEX(1, 0);
        VERTEX(1, 1);

#undef VERTEX
    };

    // Iterate over characters and add them as triangles to the vertex buffer.
    for (size_t characterIndex = 0; characterIndex < charactersCount; characterIndex++) {
        appendCharacterVertices(text[characterIndex]);
    }
    if (ellipsisNeeded) {
        appendCharacterVertices('.');
        appendCharacterVertices('.');
        appendCharacterVertices('.');
    }

    *outVertexCount = charactersCount;
    if (ellipsisNeeded) {
        *outVertexCount += 3;
    }
    *outVertexCount *= verticesInQuad;
    *outXOffset = -textWidth / 2;

    return result;
}
