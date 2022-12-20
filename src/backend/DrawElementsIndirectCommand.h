#pragma once

#include "ki/GL.h"

//#pragma pack(push, 1)
struct DrawElementsIndirectCommand
{
    GLuint count;
    GLuint instanceCount;
    GLuint firstIndex;
    GLuint baseVertex;
    GLuint baseInstance;
};
//#pragma pack(pop)
