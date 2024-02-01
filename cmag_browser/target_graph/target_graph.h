#pragma once

#include "cmag_browser/target_graph/text_renderer.h"
#include "cmag_browser/util/gl_extensions.h"
#include "cmag_core/core/cmag_project.h"

#include <array>
#include <glm/matrix.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <unordered_map>

class CmagBrowserTheme;
struct ImGuiIO;
struct ShapeInfo;
struct Segment;
struct Vec;

enum class CmakeDependencyType {
    Build = 1,
    Interface = 2,
    Additional = 4,

    NONE = 0,
    COUNT = 3,
    DEFAULT = Build | Additional,
};

inline CmakeDependencyType operator&(CmakeDependencyType a, CmakeDependencyType b) {
    return static_cast<CmakeDependencyType>(static_cast<int>(a) & static_cast<int>(b));
}

inline CmakeDependencyType operator^(CmakeDependencyType a, CmakeDependencyType b) {
    return static_cast<CmakeDependencyType>(static_cast<int>(a) ^ static_cast<int>(b));
}

class TargetGraph {
public:
    TargetGraph(const CmagBrowserTheme &theme, CmagProject &project, std::string_view defaultConfig);
    ~TargetGraph();

    void setScreenSpaceAvailableSpace(float spaceX, float spaceY);
    void setScreenSpacePosition(size_t x, size_t y);
    void setCurrentCmakeConfig(std::string_view newConfig);
    void setDisplayedDependencyType(CmakeDependencyType newType);
    void setFocusedTarget(CmagTarget *target);
    void setSelectedTarget(CmagTarget *target);
    void update(ImGuiIO &io);

    void render();
    CmagTarget *getSelectedTarget() { return selectedTarget; }
    CmagTarget *getFocusedTarget() { return focusedTarget; }

    auto getTexture() const { return framebuffer.colorTex; }
    auto getTextureWidth() const { return bounds.width; }
    auto getTextureHeight() const { return bounds.height; }

    auto getNodeScalePtr() { return &nodeScale; }
    auto getTextScalePtr() { return &textScale; }
    auto getArrowLengthScalePtr() { return &arrowLengthScale; }
    auto getArrowWidthScalePtr() { return &arrowWidthScale; }
    auto getLineStippleScalePtr() { return &lineStippleScale; }

    void refreshModelMatrices();
    void refreshConnections();
    void showEntireGraph();
    void resetGraphLayout();

private:
    struct Shapes;
    void fillTargetsVector(std::vector<CmagTarget> &allTargets);
    float calculateDepthValueForTarget(const CmagTarget &target, bool forText) const;
    static void calculateWorldSpaceVerticesForTarget(const CmagTarget &target, const Shapes &shapes, float *outVertices, size_t *outVerticesCount);

    // General data and subobjects
    const CmagBrowserTheme &theme;
    std::vector<CmagTarget *> targets = {};
    CmagTarget *selectedTarget = nullptr;
    CmagTarget *focusedTarget = nullptr;
    TextRenderer textRenderer = {};
    glm::mat4 projectionMatrix = {};
    std::string_view cmakeConfig = {};
    CmakeDependencyType displayedDependencyType = CmakeDependencyType::Build;
    float nodeScale = 25.f;
    float textScale = 3.f;
    float arrowLengthScale = 9.3f;
    float arrowWidthScale = 3.15f;
    float lineStippleScale = 0.01f;

    // Every target has a void* userData field to track custom, gui-specific data. We allocate a vector of our data structs
    // and bind them to each target.
    struct TargetData {
        struct UserData {
            glm::mat4 modelMatrix = {};
            glm::mat4 textModelMatrix = {};
        };
        std::vector<UserData> storage = {};

        void allocate(std::vector<CmagTarget *> &targets, float nodeScale, float textScale);
        void deallocate(std::vector<CmagTarget *> &targets);
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

    // The camera defines visible part of the world space and can be altered with mouse movement.
    struct Camera {
        glm::vec2 position = {0, 0};
        glm::mat4 viewMatrix = {};
        float scale = 1.f;

        bool dragActive = false;
        glm::vec4 dragOffset = {};
        glm::vec4 dragStartPos = {};

        void updateMatrix();
        void beginDrag(float mouseX, float mouseY);
        void updateDrag(float mouseX, float mouseY, const glm::mat4 &projectionMatrix);
        void endDrag();
        void zoom(bool closer);
    } camera = {};

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
        std::array<GLint, static_cast<int>(CmagTargetType::COUNT)> offsets = {};
        struct {
            GLuint vbo = {};
            GLuint vao = {};
        } gl = {};
        float maxWidth = {};
        float maxHeight = {};

        void allocate();
        void deallocate();
    } shapes = {};

    // There are connections between the targets, which graphically represent dependencies. We keep a vertex buffer and
    // rebuild it when necessary (e.g. when nodes are moved).
    struct Connections {
        struct DrawCall {
            GLenum mode = {};
            GLint offset = {};
            GLsizei count = {};
            float stippleScale = {};
            float stippleRatio = {};
            bool isFocused = {};
            bool isSelected = {};
        };
        constexpr static inline size_t maxDrawCallsCount = 12;
        size_t drawCallsCount = {};
        DrawCall drawCalls[maxDrawCallsCount] = {};
        struct {
            GLuint vbo = {};
            GLuint vao = {};
        } gl = {};

        void allocate(const std::vector<CmagTarget *> &targets);
        void deallocate();
        void update(const std::vector<CmagTarget *> &targets, std::string_view cmakeConfig, CmakeDependencyType dependencyType, const Shapes &shapes, float arrowLengthScale, float arrowWidthScale, float stippleScale, CmagTarget *focusedTarget, CmagTarget *selectedTarget);
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
            GLint stippleData = {};
            GLint color = {};
            GLint screenSize = {};
        } uniformLocation = {};

        void allocate();
        void deallocate();
    } program = {};
};
