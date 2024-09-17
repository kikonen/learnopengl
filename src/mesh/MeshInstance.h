#pragma once

#include <memory>

#include <glm/glm.hpp>

#include "ki/size.h"

namespace mesh {
    class Mesh;

    struct MeshInstance {
        glm::mat4 m_transform{ 1.f };
        std::shared_ptr<mesh::Mesh> m_mesh;
        int m_materialIndex{ -1 };
        ki::program_id m_programId{ 0 };
        bool m_shared : 1 { false };
    };
}
