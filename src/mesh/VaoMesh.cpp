#include "VaoMesh.h"

namespace mesh {
    VaoMesh::VaoMesh(std::string_view name)
        : Mesh{name}
    {}

    VaoMesh::~VaoMesh() = default;

    AABB VaoMesh::calculateAABB(const glm::mat4& transform) const
    {
        AABB aabb{ true };

        for (auto&& vertex : m_vertices)
        {
            const auto& pos = transform * glm::vec4(vertex.pos, 1.f);
            aabb.minmax(pos);
        }

        //KI_INFO_OUT(fmt::format(
        //    "AABB: model={}, min={}, max={}",
        //    m_name, aabb.m_min, aabb.m_max));

        aabb.updateVolume();

        return aabb;
    }
}
