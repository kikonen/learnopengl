#pragma once

#include <memory>

#include <glm/glm.hpp>

namespace mesh {
    class Mesh;

    struct MeshInstance {
        bool m_shared{ false };
        std::shared_ptr<mesh::Mesh> m_mesh;
        glm::mat4 m_transform{ 1.f };
        int m_materialIndex{ -1 };
    };
}
