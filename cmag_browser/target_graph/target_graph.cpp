#include "target_graph.h"

#include "cmag_browser/browser_state/cmag_browser_theme.h"
#include "cmag_browser/target_graph/coordinate_space.h"
#include "cmag_browser/target_graph/shapes.h"
#include "cmag_browser/util/gl_helpers.h"

#include <generated/fragment_shader.glsl.h>
#include <generated/vertex_shader.glsl.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <memory>

TargetGraph::TargetGraph(BrowserState &browser)
    : browser(browser) {
    fillTargetsVector(browser.getProject().getTargets());

    shapes.allocate();
    connections.allocate(targets);
    program.allocate();
    targetData.allocate(targets, nodeScale, textScale);

    projectionMatrix = glm::ortho(-worldSpaceHalfWidth, worldSpaceHalfWidth, -worldSpaceHalfHeight, worldSpaceHalfHeight);

    setCurrentCmakeConfig(browser.getConfigSelector().getCurrentConfig());

    if (browser.getProject().getGlobals().browser.needsLayout) {
        resetGraphLayout();
        browser.getProject().getGlobals().browser.needsLayout = false;
    }
    showEntireGraph();
}

TargetGraph ::~TargetGraph() {
    framebuffer.deallocate();

    targetData.deallocate(targets);
    program.deallocate();
    connections.deallocate();
    shapes.deallocate();
}

void TargetGraph::setScreenSpaceAvailableSpace(float spaceX, float spaceY) {
    FATAL_ERROR_IF(spaceX == 0 || spaceY == 0, "Zero dimensions");

    // Scale dimensions, so we keep aspect ratio
    if (spaceX > spaceY * screenSpaceAspectRatio) {
        spaceX = spaceY * screenSpaceAspectRatio;
    }
    if (spaceY > spaceX / screenSpaceAspectRatio) {
        spaceY = spaceX / screenSpaceAspectRatio;
    }

    // Early return if dimensions did not change
    const auto newWidth = static_cast<size_t>(spaceX);
    const auto newHeight = static_cast<size_t>(spaceY);
    if (newWidth == bounds.width && newHeight == bounds.height) {
        return;
    }

    // Set new dimensions
    bounds.width = newWidth;
    bounds.height = newHeight;

    // Reallocate framebuffer
    if (newWidth > 0 && newHeight > 0) {
        framebuffer.allocate(bounds.width, bounds.height);
    }
}

void TargetGraph::setScreenSpacePosition(size_t x, size_t y) {
    bounds.x = x;
    bounds.y = y;
}

#include "cmag_browser/util/graph_layout.h"

void TargetGraph::update(ImGuiIO &io) {
    // ImGuiIO gives us mouse position global to the whole window. We have to transform it so it's relative
    // to our graph. We're actually transforming it to clip space, since it will be from -1 to 1.
    const float mouseX = 2 * (io.MousePos.x - static_cast<float>(bounds.x)) / static_cast<float>(bounds.width) - 1;
    const float mouseY = 2 * (io.MousePos.y - static_cast<float>(bounds.y)) / static_cast<float>(bounds.height) - 1;
    const bool mouseInside = -1 <= mouseX && mouseX <= 1 && -1 <= mouseY && mouseY <= 1;
    const bool mouseMoved = io.MousePos.x != io.MousePosPrev.x || io.MousePos.y != io.MousePosPrev.y;

    // Calculate mouse position in world space
    const glm::mat4 vpMatrix = projectionMatrix * camera.viewMatrix;
    const glm::mat4 clipToWorldMatrix = glm::inverse(vpMatrix);
    const glm::vec4 mouseWorld = clipToWorldMatrix * glm::vec4(mouseX, mouseY, 0, 1);

    if (mouseInside && !targetDrag.active) {
        CmagTarget *currentFocusedTarget = nullptr;
        for (CmagTarget *target : targets) {
            // Vertices are in their local space. Transform mouse coordinates to this local space, so they are comparable.
            glm::mat4 clipToLocalMatrix = glm::inverse(vpMatrix * TargetData::get(*target).modelMatrix);
            glm::vec4 mouseLocal = clipToLocalMatrix * glm::vec4{mouseX, mouseY, 0, 1};

            // Check if mouse cursor is within the shape.
            const ShapeInfo *shapeInfo = shapes.shapeInfos[static_cast<int>(target->type)];
            const Vec *polygon = reinterpret_cast<const Vec *>(shapeInfo->floats);
            const size_t verticesCount = shapeInfo->floatsCount / 2;
            if (Vec{mouseLocal.x, mouseLocal.y}.isInsidePolygon(polygon, verticesCount)) {
                currentFocusedTarget = target;
            }
        }
        setFocusedTarget(currentFocusedTarget);
    } else {
        setFocusedTarget(nullptr);
    }

    focusedConnection = nullptr;
    if (mouseInside && !targetDrag.active && focusedTarget == nullptr) {
        for (ConnectionData &connection : connections.connectionsData) {
            if (Vec{mouseWorld.x, mouseWorld.y}.isInsidePolygon(connection.hoverQuad, 4)) {
                focusedConnection = &connection;
                break;
            }
        }
    }

    if (mouseMoved) {
        const bool updated = targetDrag.update(mouseX, mouseY, vpMatrix);
        if (updated) {
            TargetData::initializeModelMatrix(*targetDrag.draggedTarget, nodeScale, textScale);
            refreshConnections();
        }

        camera.updateDrag(mouseX, mouseY, projectionMatrix);
    }

    if (mouseInside && io.MouseClicked[ImGuiMouseButton_Left]) {
        setSelectedTarget(focusedTarget);
        browser.getProjectSaver().makeDirty(ProjectDirtyFlag::SelectedTarget);

        if (focusedTarget) {
            targetDrag.begin(mouseX, mouseY, vpMatrix, focusedTarget);
        } else {
            camera.beginDrag(mouseX, mouseY);
        }
    }
    if (io.MouseReleased[ImGuiMouseButton_Left]) {
        if (targetDrag.active) {
            targetDrag.end();
            browser.getProjectSaver().makeDirty(ProjectDirtyFlag::NodePosition);
        }
        if (camera.dragActive) {
            camera.endDrag();
            browser.getProjectSaver().makeDirty(ProjectDirtyFlag::CameraPosition);
        }
    }

    if (mouseInside && io.MouseWheel != 0) {
        camera.zoom(io.MouseWheel > 0);
        browser.getProjectSaver().makeDirty(ProjectDirtyFlag::CameraPosition);
    }
}

void TargetGraph::render() {
    const CmagBrowserTheme &theme = browser.getTheme();
#define THEME_COLOR(name) (&theme.name.Value.x)

    SAFE_GL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer.fbo));
    SAFE_GL(glEnable(GL_DEPTH_TEST));

    SAFE_GL(glViewport(0, 0, static_cast<GLsizei>(bounds.width), static_cast<GLsizei>(bounds.height)));
    const auto colorBackground = theme.colorTargetGraphBackground.Value;
    SAFE_GL(glClearColor(colorBackground.x, colorBackground.y, colorBackground.z, colorBackground.w));
    SAFE_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    const glm::mat4 vpMatrix = projectionMatrix * camera.viewMatrix;

    // Render targets
    SAFE_GL(glUseProgram(program.gl.program));
    SAFE_GL(glUniform2f(program.uniformLocation.screenSize, static_cast<float>(bounds.width), static_cast<float>(bounds.height)));
    SAFE_GL(glBindVertexArray(shapes.gl.vao));
    SAFE_GL(glEnableVertexAttribArray(0));
    for (const CmagTarget *target : targets) {
        const GLint vbBaseOffset = shapes.offsets[static_cast<int>(target->type)] / 2;
        const ShapeInfo &shape = *shapes.shapeInfos[static_cast<int>(target->type)];

        const auto modelMatrix = TargetData::get(*target).modelMatrix;
        const auto transform = vpMatrix * modelMatrix;
        SAFE_GL(glUniformMatrix4fv(program.uniformLocation.transform, 1, GL_FALSE, glm::value_ptr(transform)));
        SAFE_GL(glUniform2f(program.uniformLocation.stippleData, 0, 0));

        // Render solid portion of the node
        if (browser.getTargetSelection().isSelected(*target)) {
            SAFE_GL(glUniform3fv(program.uniformLocation.color, 1, THEME_COLOR(colorTargetGraphNodeSelected)));
        } else if (target == focusedTarget) {
            SAFE_GL(glUniform3fv(program.uniformLocation.color, 1, THEME_COLOR(colorTargetGraphNodeFocused)));
        } else {
            SAFE_GL(glUniform3fv(program.uniformLocation.color, 1, THEME_COLOR(colorTargetGraphNode)));
        }
        SAFE_GL(glUniform1f(program.uniformLocation.depthValue, calculateDepthValueForTarget(*target, false)));
        SAFE_GL(glDrawArrays(GL_TRIANGLE_FAN, vbBaseOffset, shape.subShapes[0].vertexCount));

        // Render outlines
        SAFE_GL(glUniform1f(program.uniformLocation.depthValue, calculateDepthValueForTarget(*target, true)));
        SAFE_GL(glUniform3fv(program.uniformLocation.color, 1, THEME_COLOR(colorTargetGraphNodeOutline)));
        for (size_t subShapeIndex = 0; subShapeIndex < shape.subShapesCount; subShapeIndex++) {
            const ShapeInfo::SubShape &subShape = shape.subShapes[subShapeIndex];
            SAFE_GL(glDrawArrays(GL_LINE_LOOP, vbBaseOffset + subShape.vertexOffset, subShape.vertexCount));
        }
    }

    // Render connections
    SAFE_GL(glUseProgram(program.gl.program));
    SAFE_GL(glBindVertexArray(connections.gl.vao));
    SAFE_GL(glEnableVertexAttribArray(0));
    SAFE_GL(glUniformMatrix4fv(program.uniformLocation.transform, 1, GL_FALSE, glm::value_ptr(vpMatrix)));
    for (size_t drawCallIndex = 0; drawCallIndex < connections.drawCallsCount; drawCallIndex++) {
        const Connections::DrawCall &drawCall = connections.drawCalls[drawCallIndex];

        const auto stippleSize = static_cast<float>(bounds.width) * drawCall.stippleScale;
        SAFE_GL(glUniform2f(program.uniformLocation.stippleData, stippleSize, drawCall.stippleRatio));

        if (drawCall.isSelected) {
            SAFE_GL(glUniform3fv(program.uniformLocation.color, 1, THEME_COLOR(colorTargetGraphConnectionSelected)));
        } else if (drawCall.isFocused) {
            SAFE_GL(glUniform3fv(program.uniformLocation.color, 1, THEME_COLOR(colorTargetGraphConnectionFocused)));
        } else {
            SAFE_GL(glUniform3fv(program.uniformLocation.color, 1, THEME_COLOR(colorTargetGraphConnection)));
        }
        SAFE_GL(glDrawArrays(drawCall.mode, drawCall.offset, drawCall.count));
    }

    // Render text
    for (const CmagTarget *target : targets) {
        auto modelMatrix = TargetData::get(*target).textModelMatrix;
        const auto transform = vpMatrix * modelMatrix;
        const auto depthValue = calculateDepthValueForTarget(*target, true);
        const auto font = ImGui::GetFont();
        textRenderer.render(transform, depthValue, target->name, font);
    }

    SAFE_GL(glDisable(GL_DEPTH_TEST));
    SAFE_GL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));

#undef THEME_COLOR
}

void TargetGraph::refreshModelMatrices() {
    for (const CmagTarget *target : targets) {
        TargetData::initializeModelMatrix(*target, nodeScale, textScale);
    }
}

void TargetGraph::fillTargetsVector(std::vector<CmagTarget> &allTargets) {
    for (CmagTarget &target : allTargets) {
        if (target.isIgnoredImportedTarget()) {
            continue;
        }

        targets.push_back(&target);
    }
}

float TargetGraph::calculateDepthValueForTarget(const CmagTarget &target, bool forText) const {
    constexpr float valueDefault = 0;
    constexpr float valueSelected = 0.1f;
    constexpr float valueFocused = 0.2f;
    constexpr float textOffset = 0.01f;

    float result = valueDefault;
    if (browser.getTargetSelection().isSelected(target)) {
        result = valueSelected;
    } else if (&target == focusedTarget) {
        result = valueFocused;
    }

    if (forText) {
        result += textOffset;
    }

    return result;
}
void TargetGraph::calculateWorldSpaceVerticesForTarget(const CmagTarget &target, const Shapes &shapes, float *outVertices, size_t *outVerticesCount) {
    const ShapeInfo *shapeInfo = shapes.shapeInfos[static_cast<int>(target.type)];
    FATAL_ERROR_IF(shapeInfo == nullptr, "Unknown shape");
    const float *targetVertices = shapeInfo->floats;
    const size_t targetVerticesSize = shapeInfo->floatsCount;

    glm::mat4 modelMatrix = TargetData::get(target).modelMatrix;
    for (size_t i = 0; i < targetVerticesSize; i += 2) {
        glm::vec4 vertex{targetVertices[i], targetVertices[i + 1], 0, 1};
        vertex = modelMatrix * vertex;
        outVertices[i + 0] = vertex.x;
        outVertices[i + 1] = vertex.y;
    }

    *outVerticesCount = targetVerticesSize;
}

void TargetGraph::refreshConnections() {
    connections.updateTopology(targets, cmakeConfig);
    connections.update(displayedDependencyType, shapes, arrowLengthScale, arrowWidthScale, lineStippleScale, focusedTarget, browser.getTargetSelection().getSelection());
}

void TargetGraph::showEntireGraph() {
    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::min();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::min();

    for (CmagTarget *target : targets) {
        const ShapeInfo *shape = shapes.shapeInfos[static_cast<int>(target->type)];
        const float targetMinX = target->graphical.x + shape->bounds.minX * nodeScale;
        const float targetMaxX = target->graphical.x + shape->bounds.maxX * nodeScale;
        const float targetMinY = target->graphical.y + shape->bounds.minY * nodeScale;
        const float targetMaxY = target->graphical.y + shape->bounds.maxY * nodeScale;

        minX = std::min(minX, targetMinX);
        maxX = std::max(maxX, targetMaxX);
        minY = std::min(minY, targetMinY);
        maxY = std::max(maxY, targetMaxY);
    }

    const float scaleX = (2 * worldSpaceHalfWidth) / (maxX - minX);
    const float scaleY = (2 * worldSpaceHalfHeight) / (maxY - minY);
    const float scale = std::min(scaleX, scaleY);
    camera.scale = scale;

    const float middleX = -scale * (maxX + minX) / 2;
    const float middleY = -scale * (maxY + minY) / 2;
    camera.position = {middleX, middleY};

    camera.updateMatrix();
}

void TargetGraph::resetGraphLayout() {
    const auto worldSpaceNodeWidth = static_cast<size_t>(shapes.maxWidth * nodeScale);
    const auto worldSpaceNodeHeight = static_cast<size_t>(shapes.maxHeight * nodeScale);

    calculateLayout(targets, cmakeConfig, worldSpaceNodeWidth, worldSpaceNodeHeight);

    refreshModelMatrices();
    refreshConnections();
}

void TargetGraph::setCurrentCmakeConfig(std::string_view newConfig) {
    if (cmakeConfig == newConfig) {
        return;
    }

    cmakeConfig = newConfig;
    refreshConnections();
}
void TargetGraph::setDisplayedDependencyType(CmagDependencyType newType) {
    if (displayedDependencyType == newType) {
        return;
    }

    displayedDependencyType = newType;
    refreshConnections();
}

void TargetGraph::setFocusedTarget(CmagTarget *target) {
    if (target == focusedTarget) {
        return;
    }
    focusedTarget = target;
    refreshConnections();
}

void TargetGraph::setSelectedTarget(CmagTarget *target) {
    if (browser.getTargetSelection().isSelected(*target)) {
        return;
    }
    browser.getTargetSelection().select(target);
    refreshConnections();
}

void TargetGraph::TargetData::allocate(std::vector<CmagTarget *> &targets, float nodeScale, float textScale) {
    storage.resize(targets.size());
    for (size_t i = 0; i < targets.size(); i++) {
        targets[i]->userData = &storage[i];
        initializeModelMatrix(*targets[i], nodeScale, textScale);
    }
}
void TargetGraph::TargetData::deallocate(std::vector<CmagTarget *> &targets) {
    for (CmagTarget *target : targets) {
        target->userData = nullptr;
    }
    storage.clear();
}
void TargetGraph::TargetData::initializeModelMatrix(const CmagTarget &target, float nodeScale, float textScale) {
    UserData &data = get(target);

    auto translationMatrix = glm::identity<glm::mat4>();
    translationMatrix = glm::translate(translationMatrix, glm::vec3(target.graphical.x, target.graphical.y, 0));

    data.modelMatrix = glm::scale(translationMatrix, glm::vec3(nodeScale, nodeScale, 1));
    data.textModelMatrix = glm::scale(translationMatrix, glm::vec3(textScale, textScale, 1));
}

TargetGraph::TargetData::UserData &TargetGraph::TargetData::get(const CmagTarget &target) {
    return *static_cast<TargetData::UserData *>(target.userData);
}

void TargetGraph::Camera::updateMatrix() {
    viewMatrix = glm::identity<glm::mat4>();
    viewMatrix = glm::translate(viewMatrix, glm::vec3(position.x, position.y, 0));
    viewMatrix = glm::translate(viewMatrix, glm::vec3(dragOffset.x, dragOffset.y, 0));
    viewMatrix = glm::scale(viewMatrix, glm::vec3{scale, scale, 1});
}

void TargetGraph::Camera::beginDrag(float mouseX, float mouseY) {
    dragActive = true;
    dragStartPos = {mouseX, mouseY, 0, 0};
    dragOffset = {};
}

void TargetGraph::Camera::updateDrag(float mouseX, float mouseY, const glm::mat4 &projectionMatrix) {
    if (!dragActive) {
        return;
    }

    dragOffset = glm::vec4(mouseX, mouseY, 0, 0) - dragStartPos;
    dragOffset = glm::inverse(projectionMatrix) * dragOffset;
    updateMatrix();
}

void TargetGraph::Camera::endDrag() {
    position.x += dragOffset.x;
    position.y += dragOffset.y;

    dragActive = false;
    dragStartPos = {};
    dragOffset = {};
}

void TargetGraph::Camera::zoom(bool closer) {
    const float interval = 0.1f;
    const float minScale = 0.1f;
    const float maxScale = 2.0f;

    if (closer) {
        scale += interval;
    } else {
        scale -= interval;
    }
    scale = clamp(scale, minScale, maxScale);
    updateMatrix();
}

void TargetGraph::TargetDrag::begin(float mouseX, float mouseY, const glm::mat4 &projectionMatrix, CmagTarget *focusedTarget) {
    active = true;
    draggedTarget = focusedTarget;

    offsetFromCenter = {mouseX, mouseY, 0, 1};
    offsetFromCenter = glm::inverse(projectionMatrix) * offsetFromCenter;
    offsetFromCenter.x -= focusedTarget->graphical.x;
    offsetFromCenter.y -= focusedTarget->graphical.y;
}

bool TargetGraph::TargetDrag::update(float mouseX, float mouseY, const glm::mat4 &projectionMatrix) const {
    if (!active) {
        return false;
    }

    glm::vec4 mouseWorld{mouseX, mouseY, 0, 1};
    mouseWorld = glm::inverse(projectionMatrix) * mouseWorld;

    draggedTarget->graphical.x = mouseWorld.x - offsetFromCenter.x;
    draggedTarget->graphical.y = mouseWorld.y - offsetFromCenter.y;

    return true;
}

void TargetGraph::TargetDrag::end() {
    *this = {};
}

void TargetGraph::Shapes::allocate() {
    // Assign shapes to target types
    shapeInfos[static_cast<int>(CmagTargetType::StaticLibrary)] = &ShapeInfo::staticLib;
    shapeInfos[static_cast<int>(CmagTargetType::ModuleLibrary)] = &ShapeInfo::moduleLib;
    shapeInfos[static_cast<int>(CmagTargetType::SharedLibrary)] = &ShapeInfo::sharedLib;
    shapeInfos[static_cast<int>(CmagTargetType::ObjectLibrary)] = &ShapeInfo::objectLib;
    shapeInfos[static_cast<int>(CmagTargetType::InterfaceLibrary)] = &ShapeInfo::interfaceLib;
    shapeInfos[static_cast<int>(CmagTargetType::UnknownLibrary)] = &ShapeInfo::unknownLib;
    shapeInfos[static_cast<int>(CmagTargetType::UnknownTarget)] = &ShapeInfo::unknownLib;
    shapeInfos[static_cast<int>(CmagTargetType::Executable)] = &ShapeInfo::executable;
    shapeInfos[static_cast<int>(CmagTargetType::Utility)] = &ShapeInfo::customTarget;

    // Sum up all vertices counts of all shapes
    size_t verticesCount = 0;
    for (const ShapeInfo *shapeInfo : shapeInfos) {
        if (shapeInfo == nullptr) {
            continue;
        }
        verticesCount += shapeInfo->floatsCount;
    }

    // Allocate one big array that will contain all the shapes and copy the vertices.
    auto data = std::make_unique<float[]>(verticesCount);
    GLsizei dataSize = 0;
    for (size_t i = 0; i < static_cast<int>(CmagTargetType::COUNT); i++) {
        const ShapeInfo *shapeInfo = shapeInfos[i];
        if (shapeInfo == nullptr) {
            continue;
        }
        memcpy(data.get() + dataSize, shapeInfo->floats, shapeInfo->floatsCount * sizeof(float));
        offsets[i] = dataSize;
        dataSize += shapeInfo->floatsCount;
    }
    dataSize *= sizeof(float);

    const GLint attribSize = 2;
    createVertexBuffer(&gl.vao, &gl.vbo, data.get(), dataSize, &attribSize, 1);

    // Calculate max bounds
    for (const ShapeInfo *shape : shapeInfos) {
        if (shape == nullptr) {
            continue;
        }
        this->maxWidth = std::max(this->maxWidth, shape->bounds.maxX - shape->bounds.minX);
        this->maxHeight = std::max(this->maxHeight, shape->bounds.maxY - shape->bounds.minY);
    }
}

void TargetGraph::Shapes::deallocate() {
    GL_DELETE_OBJECT(gl.vbo, Buffers);
    GL_DELETE_OBJECT(gl.vao, VertexArrays);
}

void TargetGraph::Connections::allocate(const std::vector<CmagTarget *> &targets) {
    // First calculate the greatest amount of connections we can have
    size_t maxConnectionsCount = 0;
    for (const CmagTarget *target : targets) {
        size_t maxCountForTarget = 0;
        for (const CmagTargetConfig &config : target->configs) {
            const size_t maxCountForConfig = config.derived.allDependencies.size();
            maxCountForTarget = std::max(maxCountForTarget, maxCountForConfig);
        }
        maxConnectionsCount += maxCountForTarget;
    }

    // Prepare shared vertex buffer for rendering all the connections. Each connection is represented by two vertices
    const size_t verticesPerConnection = 5; // line + triangle
    const GLint attribsPerVertex = 2;       // x,y
    const size_t dataSize = maxConnectionsCount * verticesPerConnection * attribsPerVertex * sizeof(float);
    createVertexBuffer(&gl.vao, &gl.vbo, nullptr, dataSize, &attribsPerVertex, 1);
}

void TargetGraph::Connections::deallocate() {
    GL_DELETE_OBJECT(gl.vbo, Buffers);
    GL_DELETE_OBJECT(gl.vao, VertexArrays);
}

void TargetGraph::Connections::updateTopology(const std::vector<CmagTarget *> &targets, std::string_view cmakeConfig) {
    connectionsData.clear();

    for (const CmagTarget *srcTarget : targets) {
        const CmagTargetConfig *config = srcTarget->tryGetConfig(cmakeConfig);
        if (config == nullptr) {
            continue;
        }

        for (const CmagTarget *dstTarget : config->derived.buildDependencies) {
            connectionsData.push_back(ConnectionData{srcTarget, dstTarget, CmagDependencyType::Build});
        }
        for (const CmagTarget *dstTarget : config->derived.interfaceDependencies) {
            connectionsData.push_back(ConnectionData{srcTarget, dstTarget, CmagDependencyType::Interface});
        }
        for (const CmagTarget *dstTarget : config->derived.manualDependencies) {
            connectionsData.push_back(ConnectionData{srcTarget, dstTarget, CmagDependencyType::Additional});
        }
    }
}

void TargetGraph::Connections::update(CmagDependencyType dependencyType, const Shapes &shapes, float arrowLengthScale, float arrowWidthScale, float stippleScale, const CmagTarget *focusedTarget, const CmagTarget *selectedTarget) {
    auto addSegment = [&](ConnectionData &connection, std::vector<float> &outLineData, std::vector<float> &outTriangleData) {
        if (connection.src->graphical.hideConnections || connection.dst->graphical.hideConnections) {
            return;
        }

        const Vec srcCenter{connection.src->graphical.x, connection.src->graphical.y};
        const Vec dstCenter{connection.dst->graphical.x, connection.dst->graphical.y};
        Segment segment{srcCenter, dstCenter};

        // Trim the connection, so it doesn't get inside the shape
        const float parameterStart = calculateSegmentTrimParameter(*connection.src, segment, shapes, true);
        const float parameterEnd = calculateSegmentTrimParameter(*connection.dst, segment, shapes, false);
        if (parameterStart >= parameterEnd) {
            return;
        }
        segment = segment.trimed(parameterStart, parameterEnd);

        // Add segment to our data
        outLineData.push_back(segment.start.x);
        outLineData.push_back(segment.start.y);
        outLineData.push_back(segment.end.x);
        outLineData.push_back(segment.end.y);

        // Add arrow
        Vec arrowA{}, arrowB{}, arrowC{};
        calculateArrowCoordinates(segment, arrowLengthScale, arrowWidthScale, arrowA, arrowB, arrowC);
        outTriangleData.push_back(arrowA.x);
        outTriangleData.push_back(arrowA.y);
        outTriangleData.push_back(arrowB.x);
        outTriangleData.push_back(arrowB.y);
        outTriangleData.push_back(arrowC.x);
        outTriangleData.push_back(arrowC.y);

        // Set hover quad
        const Vec perpendicularOffset = (segment.end - segment.start)
                                            .rotated90()
                                            .normalized()
                                            .scaled(arrowWidthScale);
        connection.hoverQuad[0] = segment.start + perpendicularOffset;
        connection.hoverQuad[1] = segment.start - perpendicularOffset;
        connection.hoverQuad[2] = segment.end - perpendicularOffset;
        connection.hoverQuad[3] = segment.end + perpendicularOffset;
    };

    struct DrawCallCandiate {
        DrawCallCandiate(GLenum mode, bool isFocused, bool isSelected, float stippleScale = 0, float stippleRatio = 0) {
            drawCall.mode = mode;
            drawCall.stippleScale = stippleScale;
            drawCall.stippleRatio = stippleRatio;
            drawCall.isFocused = isFocused;
            drawCall.isSelected = isSelected;
        }

        DrawCall drawCall = {};
        std::vector<float> data = {};
    };
    DrawCallCandiate lines{GL_LINES, false, false};
    DrawCallCandiate linesSmallStipple{GL_LINES, false, false, stippleScale / 2, 0.5f};
    DrawCallCandiate linesBigStipple{GL_LINES, false, false, stippleScale, 0.5f};
    DrawCallCandiate triangles{GL_TRIANGLES, false, false};
    DrawCallCandiate linesFocused{GL_LINES, true, false};
    DrawCallCandiate linesFocusedSmallStipple{GL_LINES, true, false, stippleScale / 2, 0.5f};
    DrawCallCandiate linesFocusedBigStipple{GL_LINES, true, false, stippleScale, 0.5f};
    DrawCallCandiate trianglesFocused{GL_TRIANGLES, true, false};
    DrawCallCandiate linesSelected{GL_LINES, false, true};
    DrawCallCandiate linesSelectedSmallStipple{GL_LINES, false, true, stippleScale / 2, 0.5f};
    DrawCallCandiate linesSelectedBigStipple{GL_LINES, false, true, stippleScale, 0.5f};
    DrawCallCandiate trianglesSelected{GL_TRIANGLES, false, true};
    DrawCallCandiate *drawCallCandidates[maxDrawCallsCount] = {
        &lines,
        &linesSmallStipple,
        &linesBigStipple,
        &triangles,
        &linesFocused,
        &linesFocusedSmallStipple,
        &linesFocusedBigStipple,
        &trianglesFocused,
        &linesSelected,
        &linesSelectedSmallStipple,
        &linesSelectedBigStipple,
        &trianglesSelected,
    };
    auto selectDrawCallCandidate = [](bool isFocused, bool isSelected, DrawCallCandiate &normal, DrawCallCandiate &focused, DrawCallCandiate &selected) -> DrawCallCandiate & {
        if (isSelected) {
            return selected;
        }
        if (isFocused) {
            return focused;
        }
        return normal;
    };

    // Loop through visible connections and add them to the vertex data
    for (ConnectionData &connection : connectionsData) {
        if (hasCmagDependencyTypeBit(connection.type, dependencyType & CmagDependencyType::Build)) {
            const bool isFocused = connection.src == focusedTarget || connection.dst == focusedTarget;
            const bool isSelected = connection.src == selectedTarget || connection.dst == selectedTarget;
            DrawCallCandiate &candidateLines = selectDrawCallCandidate(isFocused, isSelected, lines, linesFocused, linesSelected);
            DrawCallCandiate &candidateTriangles = selectDrawCallCandidate(isFocused, isSelected, triangles, trianglesFocused, trianglesSelected);
            addSegment(connection, candidateLines.data, candidateTriangles.data);
        }
        if (hasCmagDependencyTypeBit(connection.type, dependencyType & CmagDependencyType::Interface)) {
            const bool isFocused = connection.src == focusedTarget || connection.dst == focusedTarget;
            const bool isSelected = connection.src == selectedTarget || connection.dst == selectedTarget;
            DrawCallCandiate &candidateLines = selectDrawCallCandidate(isFocused, isSelected, linesBigStipple, linesFocusedBigStipple, linesSelectedBigStipple);
            DrawCallCandiate &candidateTriangles = selectDrawCallCandidate(isFocused, isSelected, triangles, trianglesFocused, trianglesSelected);
            addSegment(connection, candidateLines.data, candidateTriangles.data);
        }
        if (hasCmagDependencyTypeBit(connection.type, dependencyType & CmagDependencyType::Additional)) {
            const bool isFocused = connection.src == focusedTarget || connection.dst == focusedTarget;
            const bool isSelected = connection.src == selectedTarget || connection.dst == selectedTarget;
            DrawCallCandiate &candidateLines = selectDrawCallCandidate(isFocused, isSelected, linesSmallStipple, linesFocusedSmallStipple, linesSelectedSmallStipple);
            DrawCallCandiate &candidateTriangles = selectDrawCallCandidate(isFocused, isSelected, triangles, trianglesFocused, trianglesSelected);
            addSegment(connection, candidateLines.data, candidateTriangles.data);
        }
    }

    // Calculate offsets and counts for all draw candidates
    const size_t componentsPerVertex = 2;
    for (size_t drawCallCandidateIndex = 0; drawCallCandidateIndex < maxDrawCallsCount; drawCallCandidateIndex++) {
        DrawCallCandiate &candidate = *drawCallCandidates[drawCallCandidateIndex];
        if (drawCallCandidateIndex > 0) {
            DrawCallCandiate &previousCandidate = *drawCallCandidates[drawCallCandidateIndex - 1];
            candidate.drawCall.offset = previousCandidate.drawCall.offset + previousCandidate.drawCall.count;
        }
        candidate.drawCall.count = static_cast<GLsizei>(candidate.data.size() / componentsPerVertex);
    }

    // Upload data to the vertex buffer and register draw calls which have non-zero count.
    const size_t bytesPerVertex = componentsPerVertex * sizeof(float);
    drawCallsCount = 0;
    SAFE_GL(glBindBuffer(GL_ARRAY_BUFFER, gl.vbo));
    for (DrawCallCandiate *candidate : drawCallCandidates) {
        if (candidate->drawCall.count > 0) {
            SAFE_GL(glBufferSubData(GL_ARRAY_BUFFER, candidate->drawCall.offset * bytesPerVertex, candidate->drawCall.count * bytesPerVertex, candidate->data.data()));
            drawCalls[drawCallsCount++] = candidate->drawCall;
        }
    }
    SAFE_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

float TargetGraph::Connections::calculateSegmentTrimParameter(const CmagTarget &target, const Segment &connectionSegment, const Shapes &shapes, bool isSrcTarget) {
    // This function finds closest point of intersection between connectionSegment (a segment between centers of two targets) and
    // all edges of a given target. The result value is a parameter from 0 to 1 specifying where to trim the connection segment.

    // Get shape vertices in world space
    size_t verticesCount = 0;
    float verticesTransformed[ShapeInfo::maxVerticesCount];
    calculateWorldSpaceVerticesForTarget(target, shapes, verticesTransformed, &verticesCount);

    // Initialize the trim parameter. If given target is at a start of connection segment, we're searching for smallest possible
    // parameter value. If it's at the end, we're searching for largest parameter value.
    float result = isSrcTarget ? 1.f : 0.f;

    // Iterate over all edges of the polygon
    for (size_t i = 0; i < verticesCount; i += 2) {
        // Get the edge segment
        const size_t j = (i + 2) % verticesCount;
        const Segment polygonEdge{
            {verticesTransformed[i], verticesTransformed[i + 1]},
            {verticesTransformed[j], verticesTransformed[j + 1]},
        };

        // Check for intersection. If no intersection, then current edge is irrelevant - skip it.
        float currentParameter = 0;
        if (!connectionSegment.calculateIntersection(polygonEdge, &currentParameter)) {
            continue;
        }

        if (isSrcTarget) {
            result = std::min(result, currentParameter);
        } else {
            result = std::max(result, currentParameter);
        }
    }

    return result;
}
void TargetGraph::Connections::calculateArrowCoordinates(const Segment &connectionSegment, float arrowLength, float arrowWidth, Vec &outA, Vec &outB, Vec &outC) {
    // Calculate direction of the segment
    const float segmentLength = (connectionSegment.end - connectionSegment.start).calculateLength();
    Vec diff = (connectionSegment.end - connectionSegment.start).scaled(1 / segmentLength);

    // Our segment may be too small to insert a triangle. Scale it down to some degree, but make it disappear for very small segments.
    constexpr float scaleDownThreshold = 0.2f;
    constexpr float scaleDownRatioMultiplier = 0.8f;
    const float scaleDownRatio = segmentLength * scaleDownRatioMultiplier / arrowLength;
    if (scaleDownRatio < scaleDownThreshold) {
        // Too small, create degenerate triangles and leave
        outA = {};
        outB = {};
        outC = {};
        return;
    } else if (scaleDownRatio < 1) {
        // Too small for full size arrow, but within the threshold. Scale down the arrow.
        arrowWidth *= scaleDownRatio;
        arrowLength *= scaleDownRatio;
    }

    // Calculate offsets
    const Vec lengthwiseOffset = diff.scaled(arrowLength);
    const Vec acrossOffset = diff.scaled(arrowWidth).rotated90();

    // Output vertices
    outA = {
        connectionSegment.end.x,
        connectionSegment.end.y,
    };
    outB = {
        connectionSegment.end.x - lengthwiseOffset.x + acrossOffset.x,
        connectionSegment.end.y - lengthwiseOffset.y + acrossOffset.y,
    };
    outC = {
        connectionSegment.end.x - lengthwiseOffset.x - acrossOffset.x,
        connectionSegment.end.y - lengthwiseOffset.y - acrossOffset.y,
    };
}

void TargetGraph::Framebuffer::allocate(size_t width, size_t height) {
    FATAL_ERROR_IF(width == 0 || height == 0, "Zero dimensions");
    deallocate();

    SAFE_GL(glGenTextures(1, &colorTex));
    SAFE_GL(glBindTexture(GL_TEXTURE_2D, colorTex));
    SAFE_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    SAFE_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    SAFE_GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));

    SAFE_GL(glGenTextures(1, &depthTex));
    SAFE_GL(glBindTexture(GL_TEXTURE_2D, depthTex));
    SAFE_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    SAFE_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    SAFE_GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr));

    SAFE_GL(glGenFramebuffers(1, &fbo));
    SAFE_GL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo));
    SAFE_GL(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0));
    SAFE_GL(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0));
    SAFE_GL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
}

void TargetGraph::Framebuffer::deallocate() {
    GL_DELETE_OBJECT(fbo, Framebuffers);
    GL_DELETE_OBJECT(colorTex, Textures);
    GL_DELETE_OBJECT(depthTex, Textures);
}

void TargetGraph::Program::allocate() {
    gl.program = createProgram(vertexShaderSource, fragmentShaderSource);
    uniformLocation.depthValue = getUniformLocation(gl.program, "depthValue");
    uniformLocation.color = getUniformLocation(gl.program, "color");
    uniformLocation.stippleData = getUniformLocation(gl.program, "stippleData");
    uniformLocation.transform = getUniformLocation(gl.program, "transform");
    uniformLocation.screenSize = getUniformLocation(gl.program, "screenSize");
}

void TargetGraph::Program::deallocate() {
    if (gl.program) {
        glDeleteProgram(gl.program);
        gl.program = {};
    }
}
