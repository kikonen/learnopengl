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
        uint16_t u_materialIndex;
        uint16_t u_shapeIndex{ 0 };
    };
#pragma pack(pop)
}
