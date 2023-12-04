#include "gl_extensions.h"

#define FUNCTION(name)                                                \
    name = reinterpret_cast<decltype(name)>(getProcAddressFn(#name)); \
    if (name == nullptr)                                              \
        return false;

namespace glext {
bool initialize(GetProcAddressFn getProcAddressFn) {
    FUNCTION(glGenBuffers)
    FUNCTION(glDeleteBuffers)
    FUNCTION(glBindBuffer)
    FUNCTION(glBufferData)
    FUNCTION(glBufferSubData)
    FUNCTION(glVertexAttribPointer)
    FUNCTION(glEnableVertexAttribArray)
    FUNCTION(glDisableVertexAttribArray)

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
    FUNCTION(glUniform1f)
    FUNCTION(glUniform2f)
    FUNCTION(glUniform3f)
    FUNCTION(glUniform3fv)
    FUNCTION(glUniformMatrix4fv)

    FUNCTION(glGenFramebuffers)
    FUNCTION(glBindFramebuffer)
    FUNCTION(glFramebufferTexture2D)
    FUNCTION(glDeleteFramebuffers)

    FUNCTION(glBlendFuncSeparate)

    return true;
}
} // namespace glext

PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLDELETEBUFFERSPROC glDeleteBuffers;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLBUFFERSUBDATAPROC glBufferSubData;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;

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
PFNGLUNIFORM1FPROC glUniform1f;
PFNGLUNIFORM2FPROC glUniform2f;
PFNGLUNIFORM3FPROC glUniform3f;
PFNGLUNIFORM3FVPROC glUniform3fv;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;

PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebuffer;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;

PFNGLBLENDFUNCSEPARATEPROC glBlendFuncSeparate;