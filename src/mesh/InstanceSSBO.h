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
        GLint u_materialIndex;
    };
#pragma pack(pop)
}
