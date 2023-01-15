#pragma once

#include <glm/glm.hpp>

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
    glm::mat4 m_modelMatrix{ 1.f }; // 4 * 4 * 4 = 64
    //glm::mat4 m_normalMatrix{ 1.f }; // 4 *  3 * 4 = 48
    glm::vec4 m_objectID{ 0.f }; // 4 * 1 * 4 = 16

    float m_materialIndex{ 0 }; // 1 * 4 = 4
    float m_highlightIndex{ 0 }; // 1 * 4 = 4

    // TOTAL: 136

    int pad1;
    int pad2;

    inline void setObjectID(int objectID) {
        int r = (objectID & 0x000000FF) >> 0;
        int g = (objectID & 0x0000FF00) >> 8;
        int b = (objectID & 0x00FF0000) >> 16;

        m_objectID.r = r / 255.0f;
        m_objectID.g = g / 255.0f;
        m_objectID.b = b / 255.0f;
        m_objectID.a = 1.0f;
    }
};
#pragma pack(pop)
