#pragma once

#include <glm/glm.hpp>

#include "ki/size.h"
#include "kigl/kigl.h"

//
// SSBO entry
//
#pragma pack(push, 1)
struct DecalSSBO {
    //glm::vec4 u_transformMatrixRow0{ 1.f, 0.f, 0.f, 0.f };
    //glm::vec4 u_transformMatrixRow1{ 0.f, 1.f, 0.f, 0.f };
    //glm::vec4 u_transformMatrixRow2{ 0.f, 0.f, 1.f, 0.f };
    glm::mat4x3 u_transformMatrix;

    GLuint u_materialIndex;
    GLuint u_spriteIndex;

    int pad1;
    int pad2;

    // NOTE KI M-T matrix needed *ONLY* if non uniform scale
    inline void setTransform(
        const glm::mat4& mat)
    {
        //{
        //    const auto& c0 = mat[0];
        //    const auto& c1 = mat[1];
        //    const auto& c2 = mat[2];
        //    const auto& c3 = mat[3];

        //    u_transformMatrixRow0[0] = c0[0];
        //    u_transformMatrixRow0[1] = c1[0];
        //    u_transformMatrixRow0[2] = c2[0];
        //    u_transformMatrixRow0[3] = c3[0];

        //    u_transformMatrixRow1[0] = c0[1];
        //    u_transformMatrixRow1[1] = c1[1];
        //    u_transformMatrixRow1[2] = c2[1];
        //    u_transformMatrixRow1[3] = c3[1];

        //    u_transformMatrixRow2[0] = c0[2];
        //    u_transformMatrixRow2[1] = c1[2];
        //    u_transformMatrixRow2[2] = c2[2];
        //    u_transformMatrixRow2[3] = c3[2];
        //}
        u_transformMatrix = mat;
    }
};
#pragma pack(pop)
