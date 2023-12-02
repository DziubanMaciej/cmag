#pragma once

#include "cmag_browser/ui/target_graph/text_renderer.h"
#include "cmag_browser/util/gl_extensions.h"
#include "cmag_lib/core/cmag_project.h"

#include <array>
#include <glm/glm.hpp>
#include <unordered_map>

struct ImGuiIO;
struct ShapeInfo;

class TargetGraph {
public:
    explicit TargetGraph(std::vector<CmagTarget> &targets);
    ~TargetGraph();

    void update(ImGuiIO &io);
    void render(float spaceX, float spaceY);
    void savePosition(size_t x, size_t y);

    CmagTarget *getSelectedTarget() { return selectedTarget; }
    auto getTexture() const { return framebuffer.colorTex; }
    auto getTextureWidth() const { return bounds.width; }
    auto getTextureHeight() const { return bounds.height; }

    auto getNodeScalePtr() { return &nodeScale; }
    auto getTextScalePtr() { return &textScale; }

    void reinitializeModelMatrices();

private:
    void scaleTargetPositionsToWorldSpace();
    void clampTargetPositionToVisibleWorldSpace(CmagTarget &target) const;
    bool calculateScreenSpaceSize(float spaceX, float spaceY);
    float calculateDepthValueForTarget(const CmagTarget &target, bool forText) const;
    void initializeTargetData();

    // General data and subobjects
    std::vector<CmagTarget> &targets;
    CmagTarget *selectedTarget = nullptr;
    CmagTarget *focusedTarget = nullptr;
    TextRenderer textRenderer = {};
    glm::mat4 projectionMatrix = {};
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
    static TargetData &getTargetData(const CmagTarget &target);

    // Bounds of our graph in screen space. It determines framebuffer size and the rendering viewport. It is obtained
    // from higher level interface, which tells this class, how big it should render.
    struct {
        size_t x = {};
        size_t y = {};
        size_t width = {};
        size_t height = {};
    } bounds = {};

    // User can drag targets on the screen with a mouse. For this to work we need to keep track of some state. This is
    // enclosed in this struct.
    struct Shapes;
    struct TargetDrag {
        bool active = {};
        glm::vec4 offsetFromCenter = {};
        CmagTarget *draggedTarget = {};

        void begin(float mouseX, float mouseY, const glm::mat4 &projectionMatrix, CmagTarget *focusedTarget);
        bool update(float mouseX, float mouseY, const glm::mat4 &projectionMatrix) const;
        void end();
    } targetDrag = {};

    // Each target type may have different shape associated with it. We keep them all in a shared vertex buffer and store
    // offsets at which they start.
    struct Shapes {
        std::array<const ShapeInfo *, static_cast<int>(CmagTargetType::COUNT)> shapeInfos = {};
        std::array<size_t, static_cast<int>(CmagTargetType::COUNT)> offsets = {};
        struct {
            GLuint vbo = {};
            GLuint vao = {};
        } gl = {};

        void allocate();
        void deallocate();
    } shapes = {};

    // There are connections between the targets, which graphically represent dependencies. We keep a vertex buffer and
    // rebuild it when necessary (e.g. when nodes are moved).
    struct Connections {
        size_t count = {};
        struct {
            GLuint vbo = {};
            GLuint vao = {};
        } gl = {};

        void allocate(const std::vector<CmagTarget> &targets);
        void deallocate();
        void update(const std::vector<CmagTarget> &targets);
    } connections = {};

    // We are rendering to an offscreen texture that is later passed to ImGui. It has to be allocated with the right size.
    // This size is obtained from ImGui and it is dependent on window size.
    struct Framebuffer {
        GLuint colorTex = {};
        GLuint depthTex = {};
        GLuint fbo = {};

        void allocate(size_t width, size_t height);
        void deallocate();
    } framebuffer = {};

    // We use a custom vertex shader + fragment shader to rasterize targets and connections onto FBO.
    struct Program {
        struct {
            GLuint program = {};
        } gl = {};
        struct {
            GLint depthValue = {};
            GLint transform = {};
            GLint color = {};
        } uniformLocation = {};

        void allocate();
        void deallocate();
    } program = {};
};
