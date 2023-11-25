#include "target_graph.h"

#include "cmag_browser/ui/target_graph/coordinate_space.h"
#include "cmag_browser/util/gl_extensions.h"
#include "cmag_browser/util/gl_helpers.h"
#include "cmag_browser/util/math_utils.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>

// Vertices for shapes are specified in model space.
// Model space is defined in range <-1, 1> for both x and y.
const float verticesStaticLib[] = {
    -0.5, -0.5, // v0
    +0.5, -0.5, // v1
    +1.0, +0.0, // v2
    +0.5, +0.5, // v3
    -0.5, +0.5, // v4
};

TargetGraph::TargetGraph(std::vector<CmagTarget> &targets) : targets(targets) {
    vertices[static_cast<int>(CmagTargetType::StaticLibrary)] = verticesStaticLib;
    verticesCounts[static_cast<int>(CmagTargetType::StaticLibrary)] = sizeof(verticesStaticLib) / sizeof(float);

    allocateBuffers();
    allocateProgram();

    scaleTargetPositions();
    initializeProjectionMatrix();
    initializeTargetData();
}

TargetGraph ::~TargetGraph() {
    deallocateStorage();
    deallocateBuffers();
    deallocateProgram();
}

void TargetGraph::update(ImGuiIO &io) {
    const float mouseX = 2 * (io.MousePos.x - bounds.x) / bounds.width - 1;
    const float mouseY = 2 * (io.MousePos.y - bounds.y) / bounds.height - 1;
    const bool mouseInside = -1 <= mouseX && mouseX <= 1 && -1 <= mouseY && mouseY <= 1;

    constexpr size_t maxVerticesSize = 20;
    float verticesTransformed[maxVerticesSize];

    focusedTarget = nullptr;
    if (mouseInside && !targetDrag.active) {
        for (CmagTarget &target : targets) {
            const float *targetVertices = vertices[static_cast<int>(target.type)];
            const size_t targetVerticesSize = verticesCounts[static_cast<int>(target.type)];

            // Transform vertices
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

    if (io.MousePos.x != io.MousePosPrev.x || io.MousePos.y != io.MousePosPrev.y) {
        if (targetDrag.active) {
            glm::vec4 mouseWorld{mouseX, mouseY, 0, 1};
            mouseWorld = glm::inverse(camera.projectionMatrix) * mouseWorld;

            targetDrag.target->graphical.x = mouseWorld.x - targetDrag.offsetFromCenter.x;
            targetDrag.target->graphical.y = mouseWorld.y - targetDrag.offsetFromCenter.y;

            clampTargetPositionToVisibleWorldSpace(targetDrag.target->graphical);
            getTargetData(*targetDrag.target).initializeModelMatrix(targetDrag.target->graphical, nodeScale, textScale);
        }
    }

    if (mouseInside && io.MouseClicked[ImGuiMouseButton_Left]) {
        if (focusedTarget) {
            targetDrag.active = true;
            targetDrag.target = focusedTarget;

            targetDrag.offsetFromCenter.x = mouseX;
            targetDrag.offsetFromCenter.y = mouseY;
            targetDrag.offsetFromCenter = glm::inverse(camera.projectionMatrix) * targetDrag.offsetFromCenter;
            targetDrag.offsetFromCenter.x -= focusedTarget->graphical.x;
            targetDrag.offsetFromCenter.y -= focusedTarget->graphical.y;
        }

        selectedTarget = focusedTarget;
    }

    if (io.MouseReleased[ImGuiMouseButton_Left] && targetDrag.active) {
        targetDrag = {};
    }
}

void TargetGraph::render(float spaceX, float spaceY) {
    if (calculateScreenSpaceSize(spaceX, spaceY)) {
        allocateStorage();
    }

    SAFE_GL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gl.framebuffer));
    SAFE_GL(glBindBuffer(GL_ARRAY_BUFFER, gl.shapeVbo));
    SAFE_GL(glUseProgram(gl.program));

    SAFE_GL(glViewport(0, 0, static_cast<GLsizei>(bounds.width), static_cast<GLsizei>(bounds.height)));
    SAFE_GL(glClearColor(1, 0, 0, 1));
    SAFE_GL(glClear(GL_COLOR_BUFFER_BIT));

    SAFE_GL(glBindVertexArray(gl.shapeVao));
    SAFE_GL(glEnableVertexAttribArray(0));
    for (const CmagTarget &target : targets) {
        const auto modelMatrix = getTargetData(target).modelMatrix;
        const auto transform = camera.projectionMatrix * modelMatrix;
        SAFE_GL(glUniformMatrix4fv(gl.programUniform.transform, 1, GL_FALSE, glm::value_ptr(transform)));

        if (&target == selectedTarget) {
            SAFE_GL(glUniform3f(gl.programUniform.color, 0, 0, 1));
            SAFE_GL(glDrawArrays(GL_TRIANGLE_FAN, 0, 5));
        } else if (&target == focusedTarget) {
            SAFE_GL(glUniform3f(gl.programUniform.color, 0, 1, 0));
            SAFE_GL(glDrawArrays(GL_TRIANGLE_FAN, 0, 5));
        }

        SAFE_GL(glUniform3f(gl.programUniform.color, 0, 0, 0));
        SAFE_GL(glDrawArrays(GL_LINE_LOOP, 0, 5));
    }

    for (const CmagTarget &target : targets) {
        auto modelMatrix = getTargetData(target).textModelMatrix;
        const auto transform = camera.projectionMatrix * modelMatrix;
        textRenderer.render(transform, target.name, ImGui::GetFont());
    }

    SAFE_GL(glUseProgram(0));
    SAFE_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
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
}

void TargetGraph::clampTargetPositionToVisibleWorldSpace(CmagTargetGraphicalData &graphical) {
    const float minX = -worldSpaceHalfWidth + nodeScale;
    const float maxX = +worldSpaceHalfWidth - nodeScale;
    const float minY = -worldSpaceHalfHeight + nodeScale;
    const float maxY = +worldSpaceHalfHeight - nodeScale;

    graphical.x = clamp(graphical.x, minX, maxX);
    graphical.y = clamp(graphical.y, minY, maxY);
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

void TargetGraph::allocateStorage() {
    FATAL_ERROR_IF(bounds.width == 0 || bounds.height == 0, "Zero dimensions");
    deallocateStorage();

    SAFE_GL(glGenTextures(1, &gl.texture));
    SAFE_GL(glBindTexture(GL_TEXTURE_2D, gl.texture));
    SAFE_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    SAFE_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    SAFE_GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, static_cast<GLsizei>(bounds.width), static_cast<GLsizei>(bounds.height), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));

    SAFE_GL(glGenFramebuffers(1, &gl.framebuffer));
    SAFE_GL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gl.framebuffer));
    SAFE_GL(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gl.texture, 0));
    SAFE_GL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
}

void TargetGraph::deallocateStorage() {
    if (gl.framebuffer) {
        glDeleteFramebuffers(1, &gl.framebuffer);
    }
    if (gl.texture) {
        glDeleteTextures(1, &gl.texture);
    }
}

void TargetGraph::allocateBuffers() {
    const float *data = vertices[static_cast<int>(CmagTargetType::StaticLibrary)];
    const size_t dataSize = verticesCounts[static_cast<int>(CmagTargetType::StaticLibrary)] * sizeof(float);
    const GLint attribSize = 2;
    createVertexBuffer(&gl.shapeVao, &gl.shapeVbo, data, dataSize, &attribSize, 1);
}

void TargetGraph::deallocateBuffers() {
    if (gl.shapeVbo) {
        glDeleteBuffers(1, &gl.shapeVbo);
    }
    if (gl.shapeVao) {
        glDeleteVertexArrays(1, &gl.shapeVao);
    }
}

void TargetGraph::allocateProgram() {
    const char *vertexShaderSource = R"(
    #version 330 core
    uniform mat4 transform;
    layout(location = 0) in vec3 aPos;
    void main() {
        gl_Position = vec4(aPos, 1.0);
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
    gl.programUniform.color = getUniformLocation(gl.program, "color");
    gl.programUniform.transform = getUniformLocation(gl.program, "transform");
}

void TargetGraph::deallocateProgram() {
    glDeleteProgram(gl.program);
}

void TargetGraph::TargetData::initializeModelMatrix(CmagTargetGraphicalData graphical, float nodeScale, float textScale) {
    glm::mat4 translationMatrix = glm::identity<glm::mat4>();
    translationMatrix = glm::translate(translationMatrix, glm::vec3(graphical.x, graphical.y, 0));

    modelMatrix = glm::scale(translationMatrix, glm::vec3(nodeScale, nodeScale, nodeScale));
    textModelMatrix = glm::scale(translationMatrix, glm::vec3(textScale, textScale, textScale));
}

TargetGraph::TargetData &TargetGraph::getTargetData(const CmagTarget &target) {
    return *static_cast<TargetData *>(target.userData);
}
