#pragma once

#include <glm/glm.hpp>


// Relative to *Node*
struct NodeInstance {
    int m_entityIndex{ 0 };

    glm::vec3 m_position{ 0.f };
    glm::vec3 m_rotation{ 0.f };
    glm::vec3 m_scale{ 1.f };

    const glm::mat4& compile(const glm::mat4& parentMatrix) const;
};
