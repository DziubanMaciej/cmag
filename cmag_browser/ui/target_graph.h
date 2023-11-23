#pragma once

#include "cmag_lib/core/cmag_project.h"

#include <Windows.h>
#include <gl/GL.h>
#include <glm/glm.hpp>

struct ImGuiIO;

class TargetGraph {
public:
    TargetGraph(std::vector<CmagTarget> &targets);
    ~TargetGraph();

    void update(ImGuiIO &io);
    void render(size_t currentWidth, size_t currentHeight);
    void savePosition(size_t x, size_t y);

    CmagTarget *getSelectedTarget() { return selectedTarget; }
    auto getTexture() { return gl.texture; }

private:
    void allocateStorage(size_t newWidth, size_t newHeight);
    void deallocateStorage();
    void allocateBuffers();
    void deallocateBuffers();
    void allocateProgram();

    void deallocateProgram();

    void initializeViewMatrix();
    glm::mat4 initializeModelMatrix(const CmagTarget &target);

    std::vector<CmagTarget> &targets;
    CmagTarget *selectedTarget = nullptr;
    CmagTarget *focusedTarget = nullptr;

    const float *vertices[static_cast<int>(CmagTargetType::COUNT)] = {};
    size_t verticesCounts[static_cast<int>(CmagTargetType::COUNT)] = {};

    const float nodeScale = 25;

    struct {
        size_t x;
        size_t y;
        size_t width;
        size_t height;
    } bounds;

    struct {
        glm::mat4 viewMatrix;
    } camera;

    struct {
        bool active = {};
        glm::vec2 startPoint = {};
        CmagTarget *target = {};
        glm::vec3 offset = {};
    } targetDrag = {};

    struct {
        GLuint shapeVbo;
        GLuint shapeVao;

        GLuint program;
        struct {
            GLint transform;
            GLint color;
        } programUniform;

        GLuint texture;
        GLuint framebuffer;
    } gl;
};