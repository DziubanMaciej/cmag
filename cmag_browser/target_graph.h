#pragma once

#include "cmag_lib/core/cmag_project.h"

#include <Windows.h>
#include <gl/GL.h>

struct ImGuiIO;

class TargetGraph {
public:
    TargetGraph(std::vector<CmagTarget> &targets);
    ~TargetGraph();

    void update(ImGuiIO &io);
    void render(size_t currentWidth, size_t currentHeight);
    void savePosition(size_t x, size_t y);

    auto getTexture() { return gl.texture; }

private:
    void allocateStorage(size_t newWidth, size_t newHeight);
    void deallocateStorage();
    void allocateBuffers();
    void deallocateBuffers();
    void allocateProgram();
    static GLuint compileShader(const char *source, GLenum shaderType);
    void deallocateProgram();

    std::vector<CmagTarget> &targets;
    const CmagTarget *selectedTarget = nullptr;
    const CmagTarget *focusedTarget = nullptr;

    const float *vertices[static_cast<int>(CmagTargetType::COUNT)] = {};
    size_t verticesCounts[static_cast<int>(CmagTargetType::COUNT)] = {};

    struct {
        size_t x;
        size_t y;
        size_t width;
        size_t height;
    } bounds;

    struct {
        GLuint shapeVbo;
        GLuint shapeVao;

        GLuint program;

        GLuint texture;
        GLuint framebuffer;
    } gl;
};
