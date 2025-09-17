#pragma once

#include "kigl/kigl.h"

namespace mesh {
#pragma pack(push, 1)
    struct LodMeshSSBO {
        //glm::mat4x3 u_transformMatrix;
        glm::vec4 u_transformMatrixRow0{ 1.f, 0.f, 0.f, 0.f };
        glm::vec4 u_transformMatrixRow1{ 0.f, 1.f, 0.f, 0.f };
        glm::vec4 u_transformMatrixRow2{ 0.f, 0.f, 1.f, 0.f };

        GLuint u_entityIndex;
        GLuint u_materialIndex;

        GLuint u_baseVertex{ 0 };
        GLuint u_baseIndex{ 0 };
        GLuint u_indexCount{ 0 };

        GLint u_socketIndex{ -1 };

        float u_minDistance2{ 0.f };
        float u_maxDistance2{ 0.f };

        GLuint u_flags{ 0 };
        GLuint u_data{ 0 };

        int pad2_1;
        int pad2_2;
        //int pad2_3;

        // NOTE KI M-T matrix needed *ONLY* if non uniform scale
        inline void setTransform(
            const glm::mat4& mat)
        {
            //u_transformMatrix = mat;
            {
                const auto& c0 = mat[0];
                const auto& c1 = mat[1];
                const auto& c2 = mat[2];
                const auto& c3 = mat[3];

                u_transformMatrixRow0[0] = c0[0];
                u_transformMatrixRow0[1] = c1[0];
                u_transformMatrixRow0[2] = c2[0];
                u_transformMatrixRow0[3] = c3[0];

                u_transformMatrixRow1[0] = c0[1];
                u_transformMatrixRow1[1] = c1[1];
                u_transformMatrixRow1[2] = c2[1];
                u_transformMatrixRow1[3] = c3[1];

                u_transformMatrixRow2[0] = c0[2];
                u_transformMatrixRow2[1] = c1[2];
                u_transformMatrixRow2[2] = c2[2];
                u_transformMatrixRow2[3] = c3[2];
            }
        }

        inline void setTransform(
            const glm::vec4& row0,
            const glm::vec4& row1,
            const glm::vec4& row2)
        {
            u_transformMatrixRow0 = row0;
            u_transformMatrixRow1 = row1;
            u_transformMatrixRow2 = row2;
        }

    };
#pragma pack(pop)
}
