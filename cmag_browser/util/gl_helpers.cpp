#include "gl_helpers.h"

#include "cmag_browser/util/gl_extensions.h"

#include <memory>

void createVertexBuffer(GLuint *outVao, GLuint *outVbo, const void *data, size_t dataSize) {
    SAFE_GL(glGenVertexArrays(1, outVao));
    SAFE_GL(glBindVertexArray(*outVao));

    SAFE_GL(glGenBuffers(1, outVbo));
    SAFE_GL(glBindBuffer(GL_ARRAY_BUFFER, *outVbo));
    SAFE_GL(glBufferData(GL_ARRAY_BUFFER, dataSize, data, GL_DYNAMIC_DRAW));
    SAFE_GL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 2, reinterpret_cast<void *>(0)));

    SAFE_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    SAFE_GL(glBindVertexArray(0));
}

static GLuint compileShader(const char *source, GLenum shaderType) {
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

GLuint createProgram(const char *vertexShaderSource, const char *fragmentShaderSource) {

    GLuint vs = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    GLuint fs = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);

    GLuint program = glCreateProgram();
    CHECK_GL_ERRORS("glCreateProgram");

    SAFE_GL(glAttachShader(program, vs));
    SAFE_GL(glAttachShader(program, fs));
    SAFE_GL(glLinkProgram(program));
    SAFE_GL(glDeleteShader(vs));
    SAFE_GL(glDeleteShader(fs));

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (success == GL_FALSE) {

        GLint logSize{};
        SAFE_GL(glGetProgramInfoLog(program, 0, &logSize, nullptr));
        auto log = std::make_unique<GLchar[]>(logSize + 1);
        SAFE_GL(glGetProgramInfoLog(program, logSize + 1, nullptr, log.get()));
        FATAL_ERROR("Linking error error: ", log.get());
    }

    return program;
}

GLint getUniformLocation(GLuint program, const char *name) {
    GLint location = glGetUniformLocation(program, name);
    CHECK_GL_ERRORS("glGetUniformLocation");
    FATAL_ERROR_IF(location == -1, "Invalid uniform location returned for ", name);
    return location;
}
