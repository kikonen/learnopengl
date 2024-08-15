#pragma once

#include "kigl/kigl.h"

namespace mesh {
#pragma pack(push, 1)
    struct InstanceSSBO {
        glm::vec4 u_transformRow0{ 1.f, 0.f, 0.f, 0.f };
        glm::vec4 u_transformRow1{ 0.f, 1.f, 0.f, 0.f };
        glm::vec4 u_transformRow2{ 0.f, 0.f, 1.f, 0.f };

        GLuint u_entityIndex;
        GLuint u_materialIndex;
        GLint u_socketIndex{ -1 };
        GLuint u_flags{ 0 };

        //int pad2_1;
        //int pad2_2;
        //int pad2_3;

        // NOTE KI M-T matrix needed *ONLY* if non uniform scale
        inline void setTransform(
            const glm::mat4& mat)
        {
            u_transformRow0[0] = mat[0][0];
            u_transformRow0[1] = mat[1][0];
            u_transformRow0[2] = mat[2][0];
            u_transformRow0[3] = mat[3][0];

            u_transformRow1[0] = mat[0][1];
            u_transformRow1[1] = mat[1][1];
            u_transformRow1[2] = mat[2][1];
            u_transformRow1[3] = mat[3][1];

            u_transformRow2[0] = mat[0][2];
            u_transformRow2[1] = mat[1][2];
            u_transformRow2[2] = mat[2][2];
            u_transformRow2[3] = mat[3][2];
        }
    };
#pragma pack(pop)
}
