#pragma once

#include "ki/GL.h"

//#pragma pack(push, 1)
struct DrawArraysIndirectCommand
{
    GLuint vertexCount;
    GLuint instanceCount;
    GLuint firstVertex;
    GLuint baseInstance;
} ;
//#pragma pack(pop)
