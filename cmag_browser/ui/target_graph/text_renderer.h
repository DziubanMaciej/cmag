#pragma once

#include <Windows.h>
#include <gl/GL.h>
#include <glm/gtc/type_ptr.hpp>
#include <string_view>
#include <vector>

struct ImFont;

class TextRenderer {
public:
    TextRenderer(std::string_view text);
    TextRenderer(TextRenderer &&other);
    TextRenderer &operator=(TextRenderer &&other);
    ~TextRenderer();

    void render(glm::mat4 transform);

private:
    void allocateVertexBuffer(ImFont *font, std::string_view text);
    void allocateProgram();

    static std::vector<float> prepareVertexData(ImFont *font, std::string_view text);

    GLuint vertexCount;
    struct {
        GLuint texture;
        GLuint vbo;
        GLuint vao;

        GLuint program;
        struct {
            GLint transform;
        } programUniform;
    } gl;
};
