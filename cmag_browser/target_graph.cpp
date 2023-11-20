#include "target_graph.h"

#include "cmag_browser/gl_extensions.h"
#include "cmag_lib/utils/error.h"

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

TargetGraph::TargetGraph() {
    allocateBuffers();
    allocateProgram();
}

TargetGraph ::~TargetGraph() {
    deallocateStorage();
    deallocateBuffers();
    deallocateProgram();
}

void TargetGraph::render(size_t currentWidth, size_t currentHeight) {
    if (currentWidth != this->width || height != this->height) {
        allocateStorage(currentWidth, currentHeight);
        this->width = currentWidth;
        this->height = currentHeight;
    }

    SAFE_GL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gl.framebuffer));
    SAFE_GL(glBindBuffer(GL_ARRAY_BUFFER, gl.shapeVbo));
    SAFE_GL(glUseProgram(gl.program));

    SAFE_GL(glViewport(0, 0, currentWidth, currentHeight));
    SAFE_GL(glClearColor(1, 0, 0, 1));
    SAFE_GL(glClear(GL_COLOR_BUFFER_BIT));
    SAFE_GL(glDrawArrays(GL_LINE_LOOP, 0, 4));
    SAFE_GL(glDrawArrays(GL_LINE_LOOP, 4, 3));

    SAFE_GL(glUseProgram(0));
    SAFE_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    SAFE_GL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
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

    float data[1024] = {
        0.1, 0.1, // v0
        0.9, 0.1, // v1
        0.4, 0.4, // v1
        0.0, 0.9, // v2

        -0.1, -0.1, // v0
        -0.9, -0.1, // v1
        -0.0, -0.9, // v2
    };

    SAFE_GL(glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_DYNAMIC_DRAW));
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
    layout(location = 0) in vec3 aPos;
    void main() {
        gl_Position = vec4(aPos, 1.0);
    }
)";
    const char *fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    void main() {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0); // Black color
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
