#pragma once

#include "ki/GL.h"

#include <glm/glm.hpp>
#include <algorithm>

constexpr unsigned int ENTITY_DRAW_ELEMENT_BIT = 1;
constexpr unsigned int ENTITY_DRAW_ARRAY_BIT = 2;
constexpr unsigned int ENTITY_BILLBOARD_BIT = 4;
constexpr unsigned int ENTITY_NO_FRUSTUM_BIT = 8;

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

    GLint u_materialIndex{ 0 }; // 1 * 4 = 4
    GLuint u_highlightIndex{ 0 }; // 1 * 4 = 4

    GLuint u_objectID{ 0 }; // 1 * 1 * 4 = 4
    GLuint u_flags{ 0 }; // 1 * 4 = 4

    GLuint u_tileX{ 0 };
    GLuint u_tileY{ 0 };

    float u_rangeYmin{ 0.f };
    float u_rangeYmax{ 0.f };

    // TOTAL: 136

    //glm::vec4 pad1;
    //glm::vec4 pad2;

    inline float getMaxScale() const {
        return std::max(
            u_modelMatrix[0][0],
            std::max(u_modelMatrix[1][1], u_modelMatrix[2][2]));
    }

    inline const glm::vec4& getWorldPosition() const {
        return u_modelMatrix[3];
    }

    inline const glm::mat4& getModelMatrix() const {
        return u_modelMatrix;
    }

    inline void setModelMatrix(const glm::mat4& mat) {
        u_modelMatrix = mat;
    }

    inline void setNormalMatrix(const glm::mat3& mat) {
        u_normalMatrix0 = mat[0];
        u_normalMatrix1 = mat[1];
        u_normalMatrix2 = mat[2];
    }

    inline void adjustPosition(const glm::vec3 adjust) {
        glm::vec4& c = u_modelMatrix[3];
        c.x += adjust.x;
        c.y += adjust.y;
        c.z += adjust.z;
    }

    inline void setObjectID(int objectID) {
    //    int r = (objectID & 0x000000FF) >> 0;
    //    int g = (objectID & 0x0000FF00) >> 8;
    //    int b = (objectID & 0x00FF0000) >> 16;

    //    u_objectID.r = r / 255.0f;
    //    u_objectID.g = g / 255.0f;
    //    u_objectID.b = b / 255.0f;
    //    u_objectID.a = 1.0f;
        u_objectID = objectID;
    }
};
#pragma pack(pop)
