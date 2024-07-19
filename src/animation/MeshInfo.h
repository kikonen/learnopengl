#pragma once

#include <string>

namespace mesh {
    class ModelMesh;
}

namespace animation {
    struct MeshInfo {
        MeshInfo(const mesh::ModelMesh* mesh);

        const std::string m_name;
        const std::string m_material;
        const uint32_t m_vertexCount;
        const uint32_t m_indexCount;
    };
}
