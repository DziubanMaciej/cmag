#pragma once

#include "cmag_browser/ui/target_graph/text_renderer.h"
#include "cmag_lib/core/cmag_project.h"

#include <Windows.h>
#include <gl/GL.h>
#include <glm/glm.hpp>
#include <unordered_map>

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
    void scaleTargetPositions();
    void initializeTargetData();
    void initializeProjectionMatrix();

    void allocateStorage(size_t newWidth, size_t newHeight);
    void deallocateStorage();
    void allocateBuffers();
    void deallocateBuffers();
    void allocateProgram();
    void deallocateProgram();

    std::vector<CmagTarget> &targets;
    CmagTarget *selectedTarget = nullptr;
    CmagTarget *focusedTarget = nullptr;

    TextRenderer *getTextRenderer(const std::string &text);
    std::unordered_map<std::string, TextRenderer> textRenderers;

    const float *vertices[static_cast<int>(CmagTargetType::COUNT)] = {};
    size_t verticesCounts[static_cast<int>(CmagTargetType::COUNT)] = {};

    const float nodeScale = 25;
    const float textScale = 3;

    // Every target has a void* userData field to track custom, gui-specific data. We allocate a vector of our data structs
    // and bind them to each target.
    struct TargetData {
        glm::mat4 modelMatrix;
        glm::mat4 textModelMatrix;

        void initializeModelMatrix(CmagTargetGraphicalData graphical, glm::vec3 dragOffset, float nodeScale, float textScale);
    };
    std::vector<TargetData> targetData = {};
    TargetData &getTargetData(const CmagTarget &target);

    // Bounds of our graph in screen space. It determines framebuffer size and the rendering viewport. It is obtained
    // from higher level interface, which tells this class, how big it should render.
    struct {
        size_t x;
        size_t y;
        size_t width;
        size_t height;
    } bounds;

    struct {
        glm::mat4 projectionMatrix;
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
