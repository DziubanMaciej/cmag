#include "target_graph.h"

#include "cmag_browser/gl_extensions.h"
#include "cmag_browser/math_utils.h"
#include "cmag_lib/utils/error.h"

#include <imgui/imgui.h>

#define CHECK_GL_ERRORS(message)                                                \
    {                                                                           \
        bool anyGlError = false;                                                \
        while (true) {                                                          \
            const GLenum glError = glGetError();                                \
            if (glError == GL_NO_ERROR) {                                       \
                break;                                                          \
            } else {                                                            \
                dumpLog(std::cerr, "GL error on \"", message, "\", ", glError); \
                anyGlError = true;                                              \
            }                                                                   \
        }                                                                       \
        FATAL_ERROR_IF(anyGlError, "");                                         \
    }

#define SAFE_GL(expression) \
    expression;             \
    CHECK_GL_ERRORS(#expression)

const float verticesStaticLib[] = {
    -0.1, -0.1, // v0
    +0.1, -0.1, // v1
    +0.2, +0.0, // v2
    +0.1, +0.1, // v3
    -0.1, +0.1, // v4
};

TargetGraph::TargetGraph(std::vector<CmagTarget> &targets) : targets(targets) {
    vertices[static_cast<int>(CmagTargetType::StaticLibrary)] = verticesStaticLib;
    verticesCounts[static_cast<int>(CmagTargetType::StaticLibrary)] = sizeof(verticesStaticLib) / sizeof(float);

    allocateBuffers();
    allocateProgram();
}

TargetGraph ::~TargetGraph() {
    deallocateStorage();
    deallocateBuffers();
    deallocateProgram();
}

void TargetGraph::update(ImGuiIO &io) {
    const float mouseX = 2 * (io.MousePos.x - bounds.x) / bounds.width - 1;
    const float mouseY = 2 * (io.MousePos.y - bounds.y) / bounds.height - 1;
    if (0 > mouseX || mouseX > 1 || 0 > mouseY || mouseY > 1) {
        return;
    }

    constexpr size_t maxVerticesSize = 20;
    float verticesTransformed[maxVerticesSize];

    printf("Mouse: %f, %f\n", mouseX, mouseY);
    focusedTarget = nullptr;
    for (const CmagTarget &target : targets) {
        const float *targetVertices = vertices[static_cast<int>(target.type)];
        const size_t targetVerticesSize = verticesCounts[static_cast<int>(target.type)];

        // Transform vertices
        const float offsetX = target.graphical.x / bounds.width;
        const float offsetY = target.graphical.y / bounds.height;
        for (int i = 0; i < targetVerticesSize; i += 2) {
            verticesTransformed[i + 0] = targetVertices[i + 0] / 5 + offsetX;
            verticesTransformed[i + 1] = targetVertices[i + 1] / 5 + offsetY;
        }

        // Check focus
        if (isPointInsidePolygon(mouseX, mouseY, verticesTransformed, targetVerticesSize)) {
            focusedTarget = &target;
            printf("  Inside %s\n", target.name.c_str());
        }
    }

    if (io.MouseClicked[ImGuiMouseButton_Left]) {
        selectedTarget = focusedTarget;
    }
}

void TargetGraph::render(size_t currentWidth, size_t currentHeight) {
    if (currentWidth != bounds.width || currentHeight != bounds.height) {
        allocateStorage(currentWidth, currentHeight);
        bounds.width = currentWidth;
        bounds.height = currentHeight;
    }

    SAFE_GL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gl.framebuffer));
    SAFE_GL(glBindBuffer(GL_ARRAY_BUFFER, gl.shapeVbo));
    SAFE_GL(glUseProgram(gl.program));

    SAFE_GL(glViewport(0, 0, currentWidth, currentHeight));
    SAFE_GL(glClearColor(1, 0, 0, 1));
    SAFE_GL(glClear(GL_COLOR_BUFFER_BIT));

    const GLint locationOffset = glGetUniformLocation(gl.program, "positionOffset");
    CHECK_GL_ERRORS("glGetUniformLocation");
    const GLint locationColor = glGetUniformLocation(gl.program, "color");
    CHECK_GL_ERRORS("glGetUniformLocation");
    for (const CmagTarget &target : targets) {
        SAFE_GL(glUniform2f(locationOffset, target.graphical.x / bounds.width, target.graphical.y / bounds.height));

        if (&target == selectedTarget) {
            SAFE_GL(glUniform3f(locationColor, 0, 0, 1));
            SAFE_GL(glDrawArrays(GL_TRIANGLE_FAN, 0, 5));
        } else if (&target == focusedTarget) {
            SAFE_GL(glUniform3f(locationColor, 0, 1, 0));
            SAFE_GL(glDrawArrays(GL_TRIANGLE_FAN, 0, 5));
        }

        SAFE_GL(glUniform3f(locationColor, 0, 0, 0));
        SAFE_GL(glDrawArrays(GL_LINE_LOOP, 0, 5));
    }

    SAFE_GL(glUseProgram(0));
    SAFE_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    SAFE_GL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
}

void TargetGraph::savePosition(size_t x, size_t y) {
    bounds.x = x;
    bounds.y = y;
}

void TargetGraph::allocateStorage(size_t newWidth, size_t newHeight) {
    FATAL_ERROR_IF(newWidth == 0 || newHeight == 0, "Zero dimensions");
    deallocateStorage();

    SAFE_GL(glGenTextures(1, &gl.texture));
    SAFE_GL(glBindTexture(GL_TEXTURE_2D, gl.texture));
    SAFE_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    SAFE_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    SAFE_GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, newWidth, newHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));

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
    SAFE_GL(glGenVertexArrays(1, &gl.shapeVao));
    SAFE_GL(glBindVertexArray(gl.shapeVao));

    SAFE_GL(glGenBuffers(1, &gl.shapeVbo));
    SAFE_GL(glBindBuffer(GL_ARRAY_BUFFER, gl.shapeVbo));

    const float *data = vertices[static_cast<int>(CmagTargetType::StaticLibrary)];
    const size_t dataSize = verticesCounts[static_cast<int>(CmagTargetType::StaticLibrary)] * sizeof(float);
    SAFE_GL(glBufferData(GL_ARRAY_BUFFER, dataSize, data, GL_DYNAMIC_DRAW));
    SAFE_GL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 2, reinterpret_cast<void *>(0)));
    SAFE_GL(glEnableVertexAttribArray(0));
}

void TargetGraph::deallocateBuffers() {
    if (gl.shapeVbo) {
        glDeleteBuffers(1, &gl.shapeVbo);
    }
    if (gl.shapeVao) {
        glDeleteVertexArrays(1, &gl.shapeVao);
    }
}

GLuint TargetGraph::compileShader(const char *source, GLenum shaderType) {
    GLuint shader = glCreateShader(shaderType);
    CHECK_GL_ERRORS("glCreateShader");

    SAFE_GL(glShaderSource(shader, 1, &source, NULL));
    glCompileShader(shader);

    GLint success{};
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (success == GL_FALSE) {
        GLint logSize{};
        SAFE_GL(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize));
        auto log = std::make_unique<GLchar[]>(logSize);
        SAFE_GL(glGetShaderInfoLog(shader, logSize, NULL, log.get()));
        FATAL_ERROR("Compilation error: ", log.get());
    }

    return shader;
}

void TargetGraph::allocateProgram() {
    const char *vertexShaderSource = R"(
    #version 330 core
    uniform vec2 positionOffset;
    layout(location = 0) in vec3 aPos;
    void main() {
        gl_Position = vec4(aPos, 1.0);
        gl_Position.xy /= 5;
        gl_Position.xy += positionOffset;
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

    GLuint vs = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    GLuint fs = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);

    gl.program = glCreateProgram();
    CHECK_GL_ERRORS("glCreateProgram");

    SAFE_GL(glAttachShader(gl.program, vs));
    SAFE_GL(glAttachShader(gl.program, fs));
    SAFE_GL(glLinkProgram(gl.program));
    SAFE_GL(glDeleteShader(vs));
    SAFE_GL(glDeleteShader(fs));

    GLint success;
    glGetProgramiv(gl.program, GL_LINK_STATUS, &success);

    if (success == GL_FALSE) {

        GLint logSize{};
        SAFE_GL(glGetProgramInfoLog(gl.program, 0, &logSize, nullptr));
        auto log = std::make_unique<GLchar[]>(logSize + 1);
        SAFE_GL(glGetProgramInfoLog(gl.program, logSize + 1, nullptr, log.get()));
        FATAL_ERROR("Linking error error: ", log.get());
    }
}
void TargetGraph::deallocateProgram() {
    glDeleteProgram(gl.program);
}
