#pragma once

#if _WIN32
#include <Windows.h>
#endif
#include <GL/gl.h>
#include <gl/glext.h>

namespace glext {
using GetProcAddressFn = void *(*)(const char *);
bool initialize(GetProcAddressFn getProcAddressFn);
} // namespace glext

extern PFNGLGENBUFFERSPROC glGenBuffers;
extern PFNGLDELETEBUFFERSPROC glDeleteBuffers;
extern PFNGLBINDBUFFERPROC glBindBuffer;
extern PFNGLBUFFERDATAPROC glBufferData;
extern PFNGLBUFFERSUBDATAPROC glBufferSubData;
extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;

extern PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
extern PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray;

extern PFNGLCREATESHADERPROC glCreateShader;
extern PFNGLSHADERSOURCEPROC glShaderSource;
extern PFNGLCOMPILESHADERPROC glCompileShader;
extern PFNGLGETSHADERIVPROC glGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
extern PFNGLDELETESHADERPROC glDeleteShader;

extern PFNGLCREATEPROGRAMPROC glCreateProgram;
extern PFNGLDELETEPROGRAMPROC glDeleteProgram;
extern PFNGLATTACHSHADERPROC glAttachShader;
extern PFNGLLINKPROGRAMPROC glLinkProgram;
extern PFNGLGETPROGRAMIVPROC glGetProgramiv;
extern PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
extern PFNGLUSEPROGRAMPROC glUseProgram;

extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
extern PFNGLUNIFORM1FPROC glUniform1f;
extern PFNGLUNIFORM2FPROC glUniform2f;
extern PFNGLUNIFORM3FPROC glUniform3f;
extern PFNGLUNIFORM3FVPROC glUniform3fv;
extern PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;

extern PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
extern PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebuffer;
extern PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
extern PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;

extern PFNGLBLENDFUNCSEPARATEPROC glBlendFuncSeparate;
