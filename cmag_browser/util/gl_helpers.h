#pragma once

#include "cmag_browser/util/gl_extensions.h"
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

#define SAFE_GL(expression)          \
    do {                             \
        expression;                  \
        CHECK_GL_ERRORS(#expression) \
    } while (false)

#define GL_DELETE_OBJECT(object, objectTypeName)    \
    do {                                            \
        if (object) {                               \
            glDelete##objectTypeName(1, &(object)); \
            object = {};                            \
        }                                           \
    } while (false)

void createVertexBuffer(GLuint *outVao, GLuint *outVbo, const void *data, size_t dataSize, const GLint *attribsSizes, size_t attribSizesCount);
GLuint createProgram(const char *vertexShaderSource, const char *fragmentShaderSource);
GLint getUniformLocation(GLuint program, const char *name);