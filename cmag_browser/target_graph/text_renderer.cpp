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

TextRenderer::TextRenderer() {
    allocateProgram();
}

TextRenderer::~TextRenderer() {
    if (gl.program) {
        glDeleteProgram(gl.program);
    }
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

    SAFE_GL(glUniform1f(gl.programUniform.depthValue, depthValue));
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

    strings.emplace_back(font, text);
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

    uniform float depthValue;
    uniform mat4 transform;
    in vec2 inPos;
    in vec2 inTexCoord;

    layout(location = 0) out vec2 outTexCoord;
    void main() {
        gl_Position = vec4(inPos, depthValue, 1);
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
    gl.programUniform.depthValue = getUniformLocation(gl.program, "depthValue");
    gl.programUniform.transform = getUniformLocation(gl.program, "transform");
}

TextRenderer::PerStringData::PerStringData(ImFont *font, std::string_view text) {
    string = std::string{text};
    vertexCount = static_cast<GLuint>(text.length()) * verticesInQuad;
    allocateVertexBuffer(font, text);
}

TextRenderer::PerStringData::~PerStringData() {
    if (gl.vbo) {
        glDeleteBuffers(1, &gl.vbo);
    }
    if (gl.vao) {
        glDeleteVertexArrays(1, &gl.vao);
    }
}

void TextRenderer::PerStringData::allocateVertexBuffer(ImFont *font, std::string_view text) {
    const std::vector<float> vertexData = prepareVertexData(font, text);
    GLint attribSizes[] = {2, 2};
    createVertexBuffer(&gl.vao, &gl.vbo, vertexData.data(), vertexData.size() * sizeof(float), attribSizes, 2u);
}

std::vector<float> TextRenderer::PerStringData::prepareVertexData(ImFont *font, std::string_view text) {

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

    // Calculate width of the text
    float textWidth = 0;
    for (char character : text) {
        textWidth += font->FindGlyphNoFallback(character)->AdvanceX;
    }
    textWidth = transformToModelSpaceX(textWidth, true);

    // Center the text by making starting x position negative
    float currentX = textWidth * -0.5f;

    // Iterate over characters and add them as triangles to the vertex buffer.
    for (char character : text) {
        const ImFontGlyph *glyph = font->FindGlyphNoFallback(character);

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
    }

    return result;
}
