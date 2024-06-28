#pragma once

#include "kigl/kigl.h"

namespace mesh {
#pragma pack(push, 1)
    struct InstanceSSBO {
        //InstanceSSBO()
        //{}

        //InstanceSSBO(
        //    GLuint entityIndex,
        //    GLuint materialIndex)
        //    : u_entityIndex(entityIndex),
        //    u_materialIndex(materialIndex)
        //{}

        GLuint u_entityIndex;
        GLuint u_meshIndex;
        GLuint u_materialIndex;
        GLint u_socketIndex{ -1 };
        GLuint u_shapeIndex{ 0 };
    };
#pragma pack(pop)
}
