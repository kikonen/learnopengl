#pragma once

#include <glm/glm.hpp>

#pragma pack(push, 1)
struct BatchEntry {
    glm::mat4 m_modelMatrix;
    glm::mat3 m_normalMatrix;
    glm::vec4 m_objectID;
    float m_materialIndex;
    float m_highlightIndex;

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
