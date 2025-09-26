#include "VaoMesh.h"

#include "asset/AABBBuilder.h"

namespace mesh {
    VaoMesh::VaoMesh(std::string_view name)
        : Mesh{name}
    {}

    VaoMesh::~VaoMesh() = default;

    AABB VaoMesh::calculateAABB(const glm::mat4& transform) const
    {
        AABBBuilder builder{};

        for (auto&& vertex : m_vertices)
        {
            const auto& pos = transform * glm::vec4(vertex.pos, 1.f);
            builder.minmax(pos);
        }

        return builder.toAABB();
    }
}
