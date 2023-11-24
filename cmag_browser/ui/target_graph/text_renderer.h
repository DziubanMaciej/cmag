#pragma once

#include "cmag_browser/util/movable_primitive.h"

#include <Windows.h>
#include <gl/GL.h>
#include <glm/gtc/type_ptr.hpp>
#include <string_view>
#include <vector>

struct ImFont;

class TextRenderer {
public:
    TextRenderer();
    TextRenderer(TextRenderer &&other) = default;
    TextRenderer &operator=(TextRenderer &&other) = default;
    ~TextRenderer();

    void render(glm::mat4 transform, std::string_view text, ImFont *font);

private:
    void allocateProgram();

    struct PerStringData {
        PerStringData(ImFont *font, std::string_view text);
        PerStringData(PerStringData &&other) = default;
        PerStringData &operator=(PerStringData &&other) = default;
        ~PerStringData();

        void allocateVertexBuffer(ImFont *font, std::string_view text);
        static std::vector<float> prepareVertexData(ImFont *font, std::string_view text);

        std::string string;
        struct {
            MovablePrimitive<GLuint> vbo;
            MovablePrimitive<GLuint> vao;
        } gl;
        MovablePrimitive<GLuint> vertexCount;
    };

    PerStringData &getStringData(std::string_view text, ImFont *font);
    GLuint getTextureId(ImFont *font);

    std::vector<PerStringData> strings;

    struct {
        MovablePrimitive<GLuint> program;
        struct {
            MovablePrimitive<GLint> transform;
        } programUniform;
    } gl;
};
