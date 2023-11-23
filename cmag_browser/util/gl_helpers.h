#pragma once

#include "cmag_lib/utils/error.h"

#include <Windows.h>
#include <gl/GL.h>

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

void createVertexBuffer(GLuint *outVao, GLuint *outVbo, const void *data, size_t dataSize);
GLuint createProgram(const char *vertexShaderSource, const char *fragmentShaderSource);
GLint getUniformLocation(GLuint program, const char *name);