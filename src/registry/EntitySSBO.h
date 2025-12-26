#pragma once

#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "ki/size.h"
#include "kigl/kigl.h"


// SSBO entry
//
// NOTE KI SSBO array alignment
// - OpenGL Programming Guide, 8th Edition, page 887
// - https://stackoverflow.com/questions/23628259/how-to-properly-pad-and-align-data-in-opengl-for-std430-layout
//
// "Structure alignment is the same as the
// alignment for the biggest structure
// member, where three - component
// vectors are not rounded up to the size of
// four - component vectors.Each structure
// will start on this alignment, and its size
// will be the space needed by its
// members, according to the previous
// rules, rounded up to a multiple of the
// structure alignment."
//
// NOTE KI https://stackoverflow.com/questions/38172696/should-i-ever-use-a-vec3-inside-of-a-uniform-buffer-or-shader-storage-buffer-o
//
#pragma pack(push, 1)
struct EntitySSBO {
    // NOTE KI must align struct to 16 byte boundary
    // matrix is just N * vec4, thus vec4 is *largest*
    // NOTE KI encode in *ROW* order
    // last row is *ALWAYS* [0, 0, 0, 1]
    //glm::mat4x3 u_modelMatrix;
    glm::vec4 u_modelMatrixRow0{ 1.f, 0.f, 0.f, 0.f };
    glm::vec4 u_modelMatrixRow1{ 0.f, 1.f, 0.f, 0.f };
    glm::vec4 u_modelMatrixRow2{ 0.f, 0.f, 1.f, 0.f };

    glm::vec3 u_normalMatrix0{ 1.f, 0.f, 0.f }; // 4 *  4 * 4 = 64
    //int pad1;
    glm::vec3 u_normalMatrix1{ 0.f, 1.f, 0.f }; // 4 *  4 * 4 = 64
    //int pad2;
    glm::vec3 u_normalMatrix2{ 0.f, 0.f, 1.f }; // 4 *  4 * 4 = 64
    //int pad3;

    // center + radius
    glm::vec4 u_volume{ 0.f, 0.f, 0.f, 1.f }; // 3 * 1 * 4 = 12 => 16

    glm::vec3 u_worldScale{ 1.f };
    //int pad4_1;

    GLuint64 u_fontHandle{ 0 };

    GLuint u_objectID{ 0 }; // 1 * 1 * 4 = 4
    GLuint u_flags{ 0 }; // 1 * 4 = 4

    //GLuint u_jointBaseIndex{ 0 };

    // material tiling
    float u_tilingX{ 1.f };
    float u_tilingY{ 1.f };

    int pad1;
    int pad2;

    // NOTE KI M-T matrix needed *ONLY* if non uniform scale
    inline void setModelMatrix(
        const glm::mat4& mat,
        bool uniformScale,
        bool updateNormal)
    {
        //u_modelMatrix = mat;
        {
            const auto& c0 = mat[0];
            const auto& c1 = mat[1];
            const auto& c2 = mat[2];
            const auto& c3 = mat[3];

            u_modelMatrixRow0[0] = c0[0];
            u_modelMatrixRow0[1] = c1[0];
            u_modelMatrixRow0[2] = c2[0];
            u_modelMatrixRow0[3] = c3[0];

            u_modelMatrixRow1[0] = c0[1];
            u_modelMatrixRow1[1] = c1[1];
            u_modelMatrixRow1[2] = c2[1];
            u_modelMatrixRow1[3] = c3[1];

            u_modelMatrixRow2[0] = c0[2];
            u_modelMatrixRow2[1] = c1[2];
            u_modelMatrixRow2[2] = c2[2];
            u_modelMatrixRow2[3] = c3[2];
        }

        if (updateNormal) {
            if (uniformScale) {
                u_normalMatrix0 = mat[0];
                u_normalMatrix1 = mat[1];
                u_normalMatrix2 = mat[2];
            }
            else {
                // https://stackoverflow.com/questions/27600045/the-correct-way-to-calculate-normal-matrix
                // https://gamedev.stackexchange.com/questions/162248/correctly-transforming-normals-for-g-buffer-in-deferred-rendering
                // ???
                // "Then, for each scene object, compute their world space transforms,
                // and normal matrices. Tangent space (TBN) matrices can be computed
                // in the first pass shader.
                //
                // The normal matrix is the inverse transpose of the world space transform
                // (not object space to view space, as you would in a simpler forward rendering pipeline)."
                // ???
                const auto& normalMat = glm::inverseTranspose(glm::mat3{ mat });
                u_normalMatrix0 = normalMat[0];
                u_normalMatrix1 = normalMat[1];
                u_normalMatrix2 = normalMat[2];
            }
        }
    }

    //inline void setNormalMatrix(const glm::mat3& mat) {
    //    u_normalMatrix0 = mat[0];
    //    u_normalMatrix1 = mat[1];
    //    u_normalMatrix2 = mat[2];
    //}
};
#pragma pack(pop)
