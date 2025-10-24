#pragma once

#include <glm/glm.hpp>

#include "kigl/kigl.h"

namespace animation
{
#pragma pack(push, 1)
    struct JointTransformSSBO {
        //glm::mat4x3 u_transformMatrix;
        glm::vec4 u_transformMatrixRow0{ 1.f, 0.f, 0.f, 0.f };
        glm::vec4 u_transformMatrixRow1{ 0.f, 1.f, 0.f, 0.f };
        glm::vec4 u_transformMatrixRow2{ 0.f, 0.f, 1.f, 0.f };

        JointTransformSSBO() {}

        JointTransformSSBO(const glm::mat4& transform)
        {
            setTransform(transform);
        }

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
