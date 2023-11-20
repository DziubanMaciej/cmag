#include "gl_extensions.h"

#define FUNCTION(name)                                           \
    name = static_cast<decltype(name)>(getProcAddressFn(#name)); \
    if (name == nullptr)                                         \
        return false;

namespace glext {
bool initialize(GetProcAddressFn getProcAddressFn) {
    FUNCTION(glGenBuffers)
    FUNCTION(glDeleteBuffers)
    FUNCTION(glBindBuffer)
    FUNCTION(glBufferData)
    FUNCTION(glVertexAttribPointer)
    FUNCTION(glEnableVertexAttribArray)

    FUNCTION(glGenVertexArrays)
    FUNCTION(glDeleteVertexArrays)
    FUNCTION(glBindVertexArray)

    FUNCTION(glCreateShader)
    FUNCTION(glShaderSource)
    FUNCTION(glCompileShader)
    FUNCTION(glGetShaderiv)
    FUNCTION(glGetShaderInfoLog)
    FUNCTION(glDeleteShader)

    FUNCTION(glCreateProgram)
    FUNCTION(glDeleteProgram)
    FUNCTION(glAttachShader)
    FUNCTION(glLinkProgram)
    FUNCTION(glGetProgramiv)
    FUNCTION(glGetProgramInfoLog)
    FUNCTION(glUseProgram)

    FUNCTION(glGetUniformLocation)
    FUNCTION(glUniform2f)
    FUNCTION(glUniform3f)

    FUNCTION(glGenFramebuffers)
    FUNCTION(glBindFramebuffer)
    FUNCTION(glFramebufferTexture2D)
    FUNCTION(glDeleteFramebuffers)
    return true;
}
} // namespace glext

PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLDELETEBUFFERSPROC glDeleteBuffers;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;

PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray;

PFNGLCREATESHADERPROC glCreateShader;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLDELETESHADERPROC glDeleteShader;

PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLDELETEPROGRAMPROC glDeleteProgram;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
PFNGLUSEPROGRAMPROC glUseProgram;

PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
PFNGLUNIFORM2FPROC glUniform2f;
PFNGLUNIFORM3FPROC glUniform3f;

PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebuffer;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
