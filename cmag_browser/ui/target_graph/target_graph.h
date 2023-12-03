#pragma once

#include "cmag_browser/ui/target_graph/text_renderer.h"
#include "cmag_browser/util/gl_extensions.h"
#include "cmag_lib/core/cmag_project.h"

#include <array>
#include <glm/glm.hpp>
#include <unordered_map>

struct ImGuiIO;
struct ShapeInfo;
struct Segment;
struct Vec;

class TargetGraph {
public:
    explicit TargetGraph(std::vector<CmagTarget> &targets);
    ~TargetGraph();

    void setScreenSpaceAvailableSpace(float spaceX, float spaceY);
    void setScreenSpacePosition(size_t x, size_t y);
    void update(ImGuiIO &io);
    void render();

    CmagTarget *getSelectedTarget() { return selectedTarget; }
    auto getTexture() const { return framebuffer.colorTex; }
    auto getTextureWidth() const { return bounds.width; }
    auto getTextureHeight() const { return bounds.height; }

    auto getNodeScalePtr() { return &nodeScale; }
    auto getTextScalePtr() { return &textScale; }
    auto getArrowLengthScalePtr() { return &arrowLengthScale; }
    auto getArrowWidthScalePtr() { return &arrowWidthScale; }

    void refreshModelMatrices();
    void refreshConnections();

private:
    struct Shapes;
    void scaleTargetPositionsToWorldSpace();
    void clampTargetPositionToVisibleWorldSpace(CmagTarget &target) const;
    float calculateDepthValueForTarget(const CmagTarget &target, bool forText) const;
    static void calculateWorldSpaceVerticesForTarget(const CmagTarget &target, const Shapes &shapes, float *outVertices, size_t *outVerticesCount);

    // General data and subobjects
    std::vector<CmagTarget> &targets;
    CmagTarget *selectedTarget = nullptr;
    CmagTarget *focusedTarget = nullptr;
    TextRenderer textRenderer = {};
    glm::mat4 projectionMatrix = {};
    float nodeScale = 25;
    float textScale = 3;
    float arrowLengthScale = 9.3;
    float arrowWidthScale = 3.15;

    // Every target has a void* userData field to track custom, gui-specific data. We allocate a vector of our data structs
    // and bind them to each target.
    struct TargetData {
        struct UserData {
            glm::mat4 modelMatrix = {};
            glm::mat4 textModelMatrix = {};
        };
        std::vector<UserData> storage = {};

        void allocate(std::vector<CmagTarget> &targets, float nodeScale, float textScale);
        void deallocate(std::vector<CmagTarget> &targets);
        static void initializeModelMatrix(const CmagTarget &target, float nodeScale, float textScale);
        static UserData &get(const CmagTarget &target);
    } targetData = {};

    // Bounds of our graph in screen space. It determines framebuffer size and the rendering viewport. It is obtained
    // from higher level interface, which tells this class, how big it should render.
    struct Bounds {
        size_t x = {};
        size_t y = {};
        size_t width = {};
        size_t height = {};
    } bounds = {};

    // User can drag targets on the screen with a mouse. For this to work we need to keep track of some state. This is
    // enclosed in this struct.
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
        size_t lineDataOffset = {};
        size_t triangleDataOffset = {};
        size_t count = {};
        struct {
            GLuint vbo = {};
            GLuint vao = {};
        } gl = {};

        void allocate(const std::vector<CmagTarget> &targets);
        void deallocate();
        void update(const std::vector<CmagTarget> &targets, const Shapes &shapes, float arrowLengthScale, float arrowWidthScale);
        static float calculateSegmentTrimParameter(const CmagTarget &target, const Segment &connectionSegment, const Shapes &shapes, bool isSrcTarget);
        static void calculateArrowCoordinates(const Segment &connectionSegment, float arrowLength, float arrowWidth, Vec &outA, Vec &outB, Vec &outC);
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
