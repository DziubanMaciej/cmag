#pragma once

#include "cmag_browser/util/gl_extensions.h"
#include "cmag_browser/util/movable_primitive.h"

#include <glm/matrix.hpp>
#include <string>
#include <string_view>
#include <vector>

struct ImFont;

class TextRenderer {
public:
    TextRenderer(float nodeScale, float textScale);
    TextRenderer(TextRenderer &&other) = default;
    TextRenderer &operator=(TextRenderer &&other) = default;
    ~TextRenderer();

    void setScalesAndInvalidate(float nodeScale, float textScale);
    void render(glm::mat4 transform, float depthValue, std::string_view text, ImFont *font);

private:
    void allocateProgram();

    struct PerStringData {
        PerStringData(ImFont *font, std::string_view text, float nodeToTextScaleRatio);
        PerStringData(PerStringData &&other) = default;
        PerStringData &operator=(PerStringData &&other) = default;
        ~PerStringData();

        static std::vector<float> prepareVertexData(ImFont *font, std::string_view text, float nodeToTextScaleRatio, GLuint *outVertexCount, float *outXOffset);

        std::string string;
        struct {
            MovablePrimitive<GLuint> vbo;
            MovablePrimitive<GLuint> vao;
        } gl;
        MovablePrimitive<GLuint> vertexCount;
        MovablePrimitive<float> xOffset;
    };

    PerStringData &getStringData(std::string_view text, ImFont *font);
    GLuint getTextureId(ImFont *font);

    float nodeToTextScaleRatio = 0.0f;
    std::vector<PerStringData> strings;

    struct {
        MovablePrimitive<GLuint> program;
        struct {
            MovablePrimitive<GLint> miscValues;
            MovablePrimitive<GLint> transform;
        } programUniform;
    } gl;
};
