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
    TextRenderer(std::string_view text);
    TextRenderer(TextRenderer &&other) = default;
    TextRenderer &operator=(TextRenderer &&other) = default;
    ~TextRenderer();

    void render(glm::mat4 transform);

private:
    void allocateVertexBuffer(ImFont *font, std::string_view text);
    void allocateProgram();

    static std::vector<float> prepareVertexData(ImFont *font, std::string_view text);

    MovablePrimitive<GLuint> vertexCount;
    struct {
        MovablePrimitive<GLuint> texture;
        MovablePrimitive<GLuint> vbo;
        MovablePrimitive<GLuint> vao;

        MovablePrimitive<GLuint> program;
        struct {
            MovablePrimitive<GLint> transform;
        } programUniform;
    } gl;
};
