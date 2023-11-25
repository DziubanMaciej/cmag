#pragma once

#include "cmag_browser/ui/target_graph/text_renderer.h"
#include "cmag_browser/util/gl_extensions.h"
#include "cmag_lib/core/cmag_project.h"

#include <glm/glm.hpp>
#include <unordered_map>

struct ImGuiIO;

class TargetGraph {
public:
    TargetGraph(std::vector<CmagTarget> &targets);
    ~TargetGraph();

    void update(ImGuiIO &io);
    void render(float spaceX, float spaceY);
    void savePosition(size_t x, size_t y);

    CmagTarget *getSelectedTarget() { return selectedTarget; }
    auto getTexture() { return gl.texture; }
    auto getTextureWidth() const { return bounds.width; }
    auto getTextureHeight() const { return bounds.height; }

    auto getNodeScalePtr() { return &nodeScale; }
    auto getTextScalePtr() { return &textScale; }

    void reinitializeModelMatrices();

private:
    void clampTargetPositionToVisibleWorldSpace(CmagTargetGraphicalData &graphical);

    void scaleTargetPositions();
    void initializeTargetData();
    void initializeProjectionMatrix();

    bool calculateScreenSpaceSize(float spaceX, float spaceY);
    void allocateStorage();
    void deallocateStorage();
    void allocateBuffers();
    void deallocateBuffers();
    void allocateProgram();
    void deallocateProgram();

    std::vector<CmagTarget> &targets;
    CmagTarget *selectedTarget = nullptr;
    CmagTarget *focusedTarget = nullptr;

    TextRenderer textRenderer = {};

    const float *vertices[static_cast<int>(CmagTargetType::COUNT)] = {};
    size_t verticesCounts[static_cast<int>(CmagTargetType::COUNT)] = {};

    float nodeScale = 25;
    float textScale = 3;

    // Every target has a void* userData field to track custom, gui-specific data. We allocate a vector of our data structs
    // and bind them to each target.
    struct TargetData {
        glm::mat4 modelMatrix;
        glm::mat4 textModelMatrix;

        void initializeModelMatrix(CmagTargetGraphicalData graphical, float nodeScale, float textScale);
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
        glm::vec4 offsetFromCenter = {};
        CmagTarget *target = {};
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
