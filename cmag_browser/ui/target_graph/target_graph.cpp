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
    scaleTargetPositionsToWorldSpace();

    shapes.allocate();
    connections.allocate(targets);
    program.allocate();
    targetData.allocate(targets, nodeScale, textScale);

    projectionMatrix = glm::ortho(-worldSpaceHalfWidth, worldSpaceHalfWidth, -worldSpaceHalfHeight, worldSpaceHalfHeight);
}

TargetGraph ::~TargetGraph() {
    framebuffer.deallocate();

    targetData.deallocate(targets);
    program.deallocate();
    connections.deallocate();
    shapes.deallocate();
}

void TargetGraph::setScreenSpaceAvailableSpace(float spaceX, float spaceY) {
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

    // Set new dimensions and update internal data
    bounds.width = newWidth;
    bounds.height = newHeight;
    // refreshConnections();
    framebuffer.allocate(bounds.width, bounds.height);
}

void TargetGraph::setScreenSpacePosition(size_t x, size_t y) {
    bounds.x = x;
    bounds.y = y;
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
            glm::mat4 viewModelMatrix = projectionMatrix * TargetData::get(target).modelMatrix;
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
        const bool updated = targetDrag.update(mouseX, mouseY, projectionMatrix);
        if (updated) {
            clampTargetPositionToVisibleWorldSpace(*targetDrag.draggedTarget);
            TargetData::initializeModelMatrix(*targetDrag.draggedTarget, nodeScale, textScale);
            refreshConnections();
        }
    }

    if (mouseInside && io.MouseClicked[ImGuiMouseButton_Left]) {
        if (focusedTarget) {
            targetDrag.begin(mouseX, mouseY, projectionMatrix, focusedTarget);
        }
        selectedTarget = focusedTarget;
    }

    if (io.MouseReleased[ImGuiMouseButton_Left] && targetDrag.active) {
        targetDrag.end();
    }
}

void TargetGraph::render() {
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

        const auto modelMatrix = TargetData::get(target).modelMatrix;
        const auto transform = projectionMatrix * modelMatrix;
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
    SAFE_GL(glUniformMatrix4fv(program.uniformLocation.transform, 1, GL_FALSE, glm::value_ptr(projectionMatrix)));
    SAFE_GL(glDrawArrays(GL_LINES, connections.lineDataOffset, connections.count * 2));
    SAFE_GL(glDrawArrays(GL_TRIANGLES, connections.triangleDataOffset, connections.count * 3));
    SAFE_GL(glUseProgram(0));
    SAFE_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));

    // Render text
    for (const CmagTarget &target : targets) {
        auto modelMatrix = TargetData::get(target).textModelMatrix;
        const auto transform = projectionMatrix * modelMatrix;
        const auto depthValue = calculateDepthValueForTarget(target, true);
        const auto font = ImGui::GetFont();
        textRenderer.render(transform, depthValue, target.name, font);
    }

    SAFE_GL(glDisable(GL_DEPTH_TEST));
    SAFE_GL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
}

void TargetGraph::refreshModelMatrices() {
    for (const CmagTarget &target : targets) {
        TargetData::initializeModelMatrix(target, nodeScale, textScale);
    }
}

void TargetGraph::scaleTargetPositionsToWorldSpace() {
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
void TargetGraph::calculateWorldSpaceVerticesForTarget(const CmagTarget &target, const Shapes &shapes, float *outVertices, size_t *outVerticesCount) {
    const ShapeInfo *shapeInfo = shapes.shapeInfos[static_cast<int>(target.type)];
    const float *targetVertices = shapeInfo->vertices;
    const size_t targetVerticesSize = shapeInfo->verticesCount;

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
    connections.update(targets, cmakeConfig, shapes, arrowLengthScale, arrowWidthScale);
}

void TargetGraph::setCurrentCmakeConfig(std::string_view newConfig) {
    if (cmakeConfig == newConfig) {
        return;
    }

    cmakeConfig = newConfig;
    refreshConnections();
}

void TargetGraph::TargetData::allocate(std::vector<CmagTarget> &targets, float nodeScale, float textScale) {
    storage.resize(targets.size());
    for (size_t i = 0; i < targets.size(); i++) {
        targets[i].userData = &storage[i];
        initializeModelMatrix(targets[i], nodeScale, textScale);
    }
}
void TargetGraph::TargetData::deallocate(std::vector<CmagTarget> &targets) {
    for (CmagTarget &target : targets) {
        target.userData = nullptr;
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
        size_t currentMaxCount = 0;
        for (const CmagTargetConfig &config : target.configs) {
            currentMaxCount = std::max(currentMaxCount, config.derived.linkDependencies.size());
            currentMaxCount = std::max(currentMaxCount, config.derived.buildDependencies.size());
        }
        maxConnectionsCount += currentMaxCount;
    }

    // Each connection is represented by two vertices
    const size_t verticesPerConnection = 5; // line + triangle
    const GLint attribsPerVertex = 2;       // x,y
    const size_t dataSize = maxConnectionsCount * verticesPerConnection * attribsPerVertex * sizeof(float);
    createVertexBuffer(&gl.vao, &gl.vbo, nullptr, dataSize, &attribsPerVertex, 1);
}

void TargetGraph::Connections::deallocate() {
    GL_DELETE_OBJECT(gl.vbo, Buffers);
    GL_DELETE_OBJECT(gl.vao, VertexArrays);
}

void TargetGraph::Connections::update(const std::vector<CmagTarget> &targets, std::string_view cmakeConfig, const Shapes &shapes, float arrowLengthScale, float arrowWidthScale) {
    count = 0;

    std::vector<float> lineData = {};
    std::vector<float> triangleData = {};
    for (const CmagTarget &srcTarget : targets) {
        const CmagTargetConfig *config = srcTarget.tryGetConfig(cmakeConfig);
        if (config == nullptr) {
            continue;
        }

        const Vec srcCenter{srcTarget.graphical.x, srcTarget.graphical.y};
        for (const CmagTarget *dstTarget : config->derived.linkDependencies) {
            const Vec dstCenter{dstTarget->graphical.x, dstTarget->graphical.y};
            Segment connection{srcCenter, dstCenter};

            // Trim the connection, so it doesn't get inside the shape
            const float parameterStart = calculateSegmentTrimParameter(srcTarget, connection, shapes, true);
            const float parameterEnd = calculateSegmentTrimParameter(*dstTarget, connection, shapes, false);
            if (parameterStart >= parameterEnd) {
                continue;
            }
            trimSegment(connection, parameterStart, parameterEnd);

            // Add segment to our data
            lineData.push_back(connection.start.x);
            lineData.push_back(connection.start.y);
            lineData.push_back(connection.end.x);
            lineData.push_back(connection.end.y);

            // Add arrow
            Vec arrowA{}, arrowB{}, arrowC{};
            calculateArrowCoordinates(connection, arrowLengthScale, arrowWidthScale, arrowA, arrowB, arrowC);
            triangleData.push_back(arrowA.x);
            triangleData.push_back(arrowA.y);
            triangleData.push_back(arrowB.x);
            triangleData.push_back(arrowB.y);
            triangleData.push_back(arrowC.x);
            triangleData.push_back(arrowC.y);

            count++;
        }
    }

    const size_t lineDataSize = lineData.size() * sizeof(float);
    const size_t triangleDataSize = triangleData.size() * sizeof(float);
    lineDataOffset = 0;
    triangleDataOffset = lineDataSize / 8;

    SAFE_GL(glBindBuffer(GL_ARRAY_BUFFER, gl.vbo));
    SAFE_GL(glBufferSubData(GL_ARRAY_BUFFER, 0, lineDataSize, lineData.data()));
    SAFE_GL(glBufferSubData(GL_ARRAY_BUFFER, lineDataSize, triangleDataSize, triangleData.data()));
    SAFE_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

float TargetGraph::Connections::calculateSegmentTrimParameter(const CmagTarget &target, const Segment &connectionSegment, const Shapes &shapes, bool isSrcTarget) {
    // This function finds closest point of intersection between connectionSegment (a segment between centers of two targets) and
    // all edges of a given target. The result value is a parameter from 0 to 1 specifying where to trim the connection segment.

    // Get shape vertices in world space
    constexpr size_t maxVerticesSize = 20;
    size_t verticesCount = 0;
    float verticesTransformed[maxVerticesSize];
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
        if (!intersectSegments(connectionSegment, polygonEdge, &currentParameter)) {
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
    Vec diff = {
        connectionSegment.end.x - connectionSegment.start.x,
        connectionSegment.end.y - connectionSegment.start.y,
    };
    const float segmentLength = length(diff);
    scale(diff, 1 / segmentLength);

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
    const Vec lengthwiseOffset{
        diff.x * arrowLength,
        diff.y * arrowLength,
    };
    const Vec acrossOffset{
        diff.y * arrowWidth,
        -diff.x * arrowWidth,
    };

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
