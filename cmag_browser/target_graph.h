#pragma once

#include <Windows.h>
#include <gl/GL.h>

class TargetGraph {
public:
    TargetGraph();
    ~TargetGraph();
    void render(size_t currentWidth, size_t currentHeight);

    auto getTexture() { return gl.texture; }

private:
    void allocateStorage(size_t newWidth, size_t newHeight);
    void deallocateStorage();
    void allocateBuffers();
    void deallocateBuffers();
    void allocateProgram();
    static GLuint compileShader(const char *source, GLenum shaderType);
    void deallocateProgram();

    size_t width;
    size_t height;

    struct {
        GLuint shapeVbo;
        GLuint shapeVao;
        GLuint program;

        GLuint texture;
        GLuint framebuffer;
    } gl;
};
