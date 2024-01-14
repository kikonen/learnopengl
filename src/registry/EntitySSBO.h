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
    glm::mat4 u_modelMatrix{ 1.f }; // 4 * 4 * 4 = 64

    glm::vec3 u_normalMatrix0{ 0.f }; // 4 *  4 * 4 = 64
    int pad1;
    glm::vec3 u_normalMatrix1{ 0.f }; // 4 *  4 * 4 = 64
    int pad2;
    glm::vec3 u_normalMatrix2{ 0.f }; // 4 *  4 * 4 = 64
    int pad3;

    // center + radius
    glm::vec4 u_volume{ 0.f }; // 3 * 1 * 4 = 12 => 16

    glm::vec3 u_worldScale{ 0.f };
    int pad4_1;

    GLint u_materialIndex{ 0 }; // 1 * 4 = 4
    GLint u_shapeIndex{ -1 }; // 1 * 4 = 4
    GLuint u_highlightIndex{ 0 }; // 1 * 4 = 4

    GLuint u_objectID{ 0 }; // 1 * 1 * 4 = 4
    GLuint u_flags{ 0 }; // 1 * 4 = 4

    GLuint u_tileX{ 0 };
    GLuint u_tileY{ 0 };

    float u_rangeYmin{ 0.f };
    float u_rangeYmax{ 0.f };

    int pad2_1;
    int pad2_2;
    int pad2_3;

    // NOTE KI maxScale *CANNOT* be get from modelmatrix if both
    // Scale AND Rotation is applied!
    inline float getMaxScale() const {
        return std::max(std::max(u_worldScale.x, u_worldScale.y), u_worldScale.z);
    }

    inline const glm::vec4& getWorldPosition() const {
        return u_modelMatrix[3];
    }

    inline const glm::mat4& getModelMatrix() const {
        return u_modelMatrix;
    }

    // NOTE KI M-T matrix needed *ONLY* if non uniform scale
    inline void setModelMatrix(
        const glm::mat4& mat,
        bool uniformScale,
        bool updateNormal)
    {
        u_modelMatrix = mat;

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
                const auto& normalMat = glm::inverseTranspose(mat);
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

    //inline void adjustPosition(const glm::vec3 adjust) {
    //    glm::vec4& c = u_modelMatrix[3];
    //    c.x += adjust.x;
    //    c.y += adjust.y;
    //    c.z += adjust.z;
    //}
};
#pragma pack(pop)
