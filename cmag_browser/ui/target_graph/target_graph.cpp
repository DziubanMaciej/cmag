#include "target_graph.h"

#include "cmag_browser/ui/target_graph/coordinate_space.h"
#include "cmag_browser/ui/target_graph/shapes.h"
#include "cmag_browser/util/gl_helpers.h"
#include "cmag_browser/util/math_utils.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <memory>

TargetGraph::TargetGraph(std::vector<CmagTarget> &targets) : targets(targets) {
    shapes.allocate();
    connections.allocate(targets);
    program.allocate();

    scaleTargetPositions();
    initializeProjectionMatrix();
    initializeTargetData();
}

TargetGraph ::~TargetGraph() {
    framebuffer.deallocate();

    program.deallocate();
    connections.deallocate();
    shapes.deallocate();
}

void TargetGraph::update(ImGuiIO &io) {
    // ImGuiIO gives us mouse position global to the whole window. We have to transform it so it's relative
    // to our graph that we're rendering.
    const float mouseX = 2 * (io.MousePos.x - static_cast<float>(bounds.x)) / static_cast<float>(bounds.width) - 1;
    const float mouseY = 2 * (io.MousePos.y - static_cast<float>(bounds.y)) / static_cast<float>(bounds.height) - 1;
    const bool mouseInside = -1 <= mouseX && mouseX <= 1 && -1 <= mouseY && mouseY <= 1;
    const bool mouseMoved = io.MousePos.x != io.MousePosPrev.x || io.MousePos.y != io.MousePosPrev.y;

    constexpr size_t maxVerticesSize = 20;
    float verticesTransformed[maxVerticesSize];

    focusedTarget = nullptr;
    if (mouseInside && !targetDrag.active) {
        for (CmagTarget &target : targets) {
            const ShapeInfo *shapeInfo = shapes.shapeInfos[static_cast<int>(target.type)];
            const float *targetVertices = shapeInfo->vertices;
            const size_t targetVerticesSize = shapeInfo->verticesCount;

            // Transform vertices from local space to screen space, so we're able to compare it with mouse position.
            // TODO wouldn't it be possible/better to transform mouse position to local space of each target?
            glm::mat4 viewModelMatrix = camera.projectionMatrix * getTargetData(target).modelMatrix;
            for (size_t i = 0; i < targetVerticesSize; i += 2) {
                glm::vec4 vertex{targetVertices[i], targetVertices[i + 1], 0, 1};
                vertex = viewModelMatrix * vertex;
                verticesTransformed[i + 0] = vertex.x;
                verticesTransformed[i + 1] = vertex.y;
            }

            // Check focus
            if (isPointInsidePolygon(mouseX, mouseY, verticesTransformed, targetVerticesSize)) {
                focusedTarget = &target;
            }
        }
    }

    if (mouseMoved) {
        const bool updated = targetDrag.update(mouseX, mouseY, camera.projectionMatrix);
        if (updated) {
            clampTargetPositionToVisibleWorldSpace(*targetDrag.draggedTarget);
            getTargetData(*targetDrag.draggedTarget).initializeModelMatrix(targetDrag.draggedTarget->graphical, nodeScale, textScale);
            connections.update(targets);
        }
    }

    if (mouseInside && io.MouseClicked[ImGuiMouseButton_Left]) {
        if (focusedTarget) {
            targetDrag.begin(mouseX, mouseY, camera.projectionMatrix, focusedTarget);
        }
        selectedTarget = focusedTarget;
    }

    if (io.MouseReleased[ImGuiMouseButton_Left] && targetDrag.active) {
        targetDrag.end();
    }
}

void TargetGraph::render(float spaceX, float spaceY) {
    if (calculateScreenSpaceSize(spaceX, spaceY)) {
        framebuffer.allocate(bounds.width, bounds.height);
    }

    SAFE_GL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer.fbo));
    SAFE_GL(glEnable(GL_DEPTH_TEST));

    SAFE_GL(glViewport(0, 0, static_cast<GLsizei>(bounds.width), static_cast<GLsizei>(bounds.height)));
    SAFE_GL(glClearColor(1, 0, 0, 1));
    SAFE_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    // Render targets
    SAFE_GL(glUseProgram(program.gl.program));
    SAFE_GL(glBindVertexArray(shapes.gl.vao));
    SAFE_GL(glEnableVertexAttribArray(0));
    for (const CmagTarget &target : targets) {
        const size_t vbOffset = shapes.offsets[static_cast<int>(target.type)] / 2;
        const size_t vbSize = shapes.shapeInfos[static_cast<int>(target.type)]->verticesCount / 2;

        const auto modelMatrix = getTargetData(target).modelMatrix;
        const auto transform = camera.projectionMatrix * modelMatrix;
        SAFE_GL(glUniformMatrix4fv(program.uniformLocation.transform, 1, GL_FALSE, glm::value_ptr(transform)));
        SAFE_GL(glUniform1f(program.uniformLocation.depthValue, calculateDepthValueForTarget(target, false)));

        if (&target == selectedTarget) {
            SAFE_GL(glUniform3f(program.uniformLocation.color, 0, 0, 1));
            SAFE_GL(glDrawArrays(GL_TRIANGLE_FAN, vbOffset, vbSize));
        } else if (&target == focusedTarget) {
            SAFE_GL(glUniform3f(program.uniformLocation.color, 0, 1, 0));
            SAFE_GL(glDrawArrays(GL_TRIANGLE_FAN, vbOffset, vbSize));
        }

        SAFE_GL(glUniform3f(program.uniformLocation.color, 0, 0, 0));
        SAFE_GL(glDrawArrays(GL_LINE_LOOP, vbOffset, vbSize));
    }
    SAFE_GL(glUseProgram(0));
    SAFE_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));

    // Render connections
    SAFE_GL(glUseProgram(program.gl.program));
    SAFE_GL(glBindVertexArray(connections.gl.vao));
    SAFE_GL(glEnableVertexAttribArray(0));
    SAFE_GL(glUniformMatrix4fv(program.uniformLocation.transform, 1, GL_FALSE, glm::value_ptr(camera.projectionMatrix)));
    SAFE_GL(glDrawArrays(GL_LINES, 0, connections.count * 2));
    SAFE_GL(glUseProgram(0));
    SAFE_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));

    // Render text
    for (const CmagTarget &target : targets) {
        auto modelMatrix = getTargetData(target).textModelMatrix;
        const auto transform = camera.projectionMatrix * modelMatrix;
        const auto depthValue = calculateDepthValueForTarget(target, true);
        const auto font = ImGui::GetFont();
        textRenderer.render(transform, depthValue, target.name, font);
    }

    SAFE_GL(glDisable(GL_DEPTH_TEST));
    SAFE_GL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
}

void TargetGraph::savePosition(size_t x, size_t y) {
    bounds.x = x;
    bounds.y = y;
}

void TargetGraph::reinitializeModelMatrices() {
    for (const CmagTarget &target : targets) {
        getTargetData(target).initializeModelMatrix(target.graphical, nodeScale, textScale);
    }
    connections.update(targets);
}

void TargetGraph::clampTargetPositionToVisibleWorldSpace(CmagTarget &target) const {
    const ShapeInfo *shape = shapes.shapeInfos[static_cast<int>(target.type)];

    const float minX = -shape->bounds.minX * nodeScale - worldSpaceHalfWidth;
    const float maxX = -shape->bounds.maxX * nodeScale + worldSpaceHalfWidth;
    const float minY = -shape->bounds.minY * nodeScale - worldSpaceHalfHeight;
    const float maxY = -shape->bounds.maxY * nodeScale + worldSpaceHalfHeight;

    target.graphical.x = clamp(target.graphical.x, minX, maxX);
    target.graphical.y = clamp(target.graphical.y, minY, maxY);
}

float TargetGraph::calculateDepthValueForTarget(const CmagTarget &target, bool forText) const {
    constexpr float valueDefault = 0;
    constexpr float valueSelected = 0.1;
    constexpr float valueFocused = 0.2;
    constexpr float textOffset = 0.01;

    float result = valueDefault;
    if (&target == selectedTarget) {
        result = valueSelected;
    } else if (&target == focusedTarget) {
        result = valueFocused;
    }

    if (forText) {
        result += textOffset;
    }

    return result;
}

void TargetGraph::scaleTargetPositions() {
    // Target positions were calculated by core cmag and the may be in an arbitrary space. We scale them to our
    // custom world space.

    // Find out bounds of arbitrary space of the targets.
    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::min();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::min();
    for (const CmagTarget &target : targets) {
        minX = std::min(minX, target.graphical.x);
        maxX = std::max(maxX, target.graphical.x);

        minY = std::min(minY, target.graphical.y);
        maxY = std::max(maxY, target.graphical.y);
    }

    // Our nodes have their size. We have to add it to our bounds, so entire node is always visible.
    minX -= nodeScale;
    maxX += nodeScale;
    minY -= nodeScale;
    maxY += nodeScale;

    // We add some additional padding.
    constexpr float paddingPercentage = 0.1f;
    const float paddingX = (maxX - minX) * paddingPercentage / 2;
    minX -= paddingX;
    maxX += paddingX;
    const float paddingY = (maxY - minY) * paddingPercentage / 2;
    minY -= paddingY;
    maxY += paddingY;

    // Linearly transform x and y of all targets to our world space.
    for (CmagTarget &target : targets) {
        target.graphical.x = interpolate(target.graphical.x, minX, maxX, -worldSpaceHalfWidth, worldSpaceHalfWidth);
        target.graphical.y = interpolate(target.graphical.y, minY, maxY, -worldSpaceHalfHeight, worldSpaceHalfHeight);
    }
}

void TargetGraph::initializeTargetData() {
    targetData.resize(targets.size());
    for (size_t i = 0; i < targets.size(); i++) {
        targets[i].userData = &targetData[i];
        targetData[i].initializeModelMatrix(targets[i].graphical, nodeScale, textScale);
    }
    connections.update(targets);
}

void TargetGraph::initializeProjectionMatrix() {
    camera.projectionMatrix = glm::ortho(-worldSpaceHalfWidth, worldSpaceHalfWidth, -worldSpaceHalfHeight, worldSpaceHalfHeight);
}

bool TargetGraph::calculateScreenSpaceSize(float spaceX, float spaceY) {

    if (spaceX > spaceY * screenSpaceAspectRatio) {
        spaceX = spaceY * screenSpaceAspectRatio;
    }
    if (spaceY > spaceX / screenSpaceAspectRatio) {
        spaceY = spaceX / screenSpaceAspectRatio;
    }

    const auto newWidth = static_cast<size_t>(spaceX);
    const auto newHeight = static_cast<size_t>(spaceY);

    if (newWidth != bounds.width || newHeight != bounds.height) {
        bounds.width = newWidth;
        bounds.height = newHeight;
        return true;
    }
    return false;
}

void TargetGraph::TargetDrag::begin(float mouseX, float mouseY, const glm::mat4 &projectionMatrix, CmagTarget *focusedTarget) {
    active = true;
    draggedTarget = focusedTarget;

    offsetFromCenter.x = mouseX;
    offsetFromCenter.y = mouseY;
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
    shapeInfos[static_cast<int>(CmagTargetType::StaticLibrary)] = &ShapeInfo::postcard;
    shapeInfos[static_cast<int>(CmagTargetType::Executable)] = &ShapeInfo::square;

    // Sum up all vertices counts of all shapes
    size_t verticesCount = 0;
    for (const ShapeInfo *shapeInfo : shapeInfos) {
        if (shapeInfo == nullptr) {
            continue;
        }
        verticesCount += shapeInfo->verticesCount;
    }

    // Allocate one big array that will contain all the shapes and copy the vertices.
    auto data = std::make_unique<float[]>(verticesCount);
    size_t dataSize = 0;
    for (size_t i = 0; i < static_cast<int>(CmagTargetType::COUNT); i++) {
        const ShapeInfo *shapeInfo = shapeInfos[i];
        if (shapeInfo == nullptr) {
            continue;
        }
        memcpy(data.get() + dataSize, shapeInfo->vertices, shapeInfo->verticesCount * sizeof(float));
        offsets[i] = dataSize;
        dataSize += shapeInfo->verticesCount;
    }
    dataSize *= sizeof(float);

    const GLint attribSize = 2;
    createVertexBuffer(&gl.vao, &gl.vbo, data.get(), dataSize, &attribSize, 1);
}

void TargetGraph::Shapes::deallocate() {
    GL_DELETE_OBJECT(gl.vbo, Buffers);
    GL_DELETE_OBJECT(gl.vao, VertexArrays);
}

void TargetGraph::Connections::allocate(const std::vector<CmagTarget> &targets) {
    // First calculate the greatest amount of connections we can have
    size_t maxConnectionsCount = 0;
    for (const CmagTarget &target : targets) {
        for (const CmagTargetConfig &config : target.configs) {
            auto currentMaxCount = std::max(config.derived.linkDependencies.size(), config.derived.buildDependencies.size());
            maxConnectionsCount += currentMaxCount;
        }
    }

    // Each connection is represented by two vertices
    const size_t verticesPerConnection = 2; // we're rendering lines
    const GLint attribsPerVertex = 2;       // x,y
    const size_t dataSize = maxConnectionsCount * verticesPerConnection * attribsPerVertex;
    createVertexBuffer(&gl.vao, &gl.vbo, nullptr, dataSize, &attribsPerVertex, 1);
}

void TargetGraph::Connections::deallocate() {
    GL_DELETE_OBJECT(gl.vbo, Buffers);
    GL_DELETE_OBJECT(gl.vao, VertexArrays);
}

void TargetGraph::Connections::update(const std::vector<CmagTarget> &targets) {
    count = 0;

    std::vector<float> data = {};
    for (const CmagTarget &srcTarget : targets) {
        const CmagTargetConfig *config = srcTarget.tryGetConfig("Debug"); // TODO make this selectable from gui
        if (config == nullptr) {
            continue;
        }

        for (const CmagTarget *dstTarget : config->derived.linkDependencies) {
            data.push_back(srcTarget.graphical.x);
            data.push_back(srcTarget.graphical.y);

            data.push_back(dstTarget->graphical.x);
            data.push_back(dstTarget->graphical.y);

            count++;
        }
    }

    const size_t dataSize = data.size() * sizeof(float);
    SAFE_GL(glBindBuffer(GL_ARRAY_BUFFER, gl.vbo));
    SAFE_GL(glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, data.data()));
    SAFE_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
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
    GL_DELETE_OBJECT(fbo, Framebuffers)
    GL_DELETE_OBJECT(colorTex, Textures)
    GL_DELETE_OBJECT(depthTex, Textures)
}
void TargetGraph::Program::allocate() {
    const char *vertexShaderSource = R"(
    #version 330 core
    uniform float depthValue;
    uniform mat4 transform;
    layout(location = 0) in vec2 aPos;
    void main() {
        gl_Position = vec4(aPos, depthValue, 1.0);
        gl_Position = transform * gl_Position;
    }
)";
    const char *fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    uniform vec3 color;
    void main() {
        FragColor = vec4(color, 1.0);
    }
)";
    gl.program = createProgram(vertexShaderSource, fragmentShaderSource);
    uniformLocation.depthValue = getUniformLocation(gl.program, "depthValue");
    uniformLocation.color = getUniformLocation(gl.program, "color");
    uniformLocation.transform = getUniformLocation(gl.program, "transform");
}

void TargetGraph::Program::deallocate() {
    if (gl.program) {
        glDeleteProgram(gl.program);
        gl.program = {};
    }
}

void TargetGraph::TargetData::initializeModelMatrix(CmagTargetGraphicalData graphical, float nodeScale, float textScale) {
    auto translationMatrix = glm::identity<glm::mat4>();
    translationMatrix = glm::translate(translationMatrix, glm::vec3(graphical.x, graphical.y, 0));

    modelMatrix = glm::scale(translationMatrix, glm::vec3(nodeScale, nodeScale, 1));
    textModelMatrix = glm::scale(translationMatrix, glm::vec3(textScale, textScale, 1));
}

TargetGraph::TargetData &TargetGraph::getTargetData(const CmagTarget &target) {
    return *static_cast<TargetData *>(target.userData);
}
