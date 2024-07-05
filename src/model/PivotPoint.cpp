#include "PivotPoint.h"

#include "mesh/MeshType.h"


glm::vec3 PivotPoint::resolve(mesh::MeshType* type) const
{
    glm::vec3 pivot{ 0.f };

    const auto& aabb = type->getAABB();
    const auto& volume = aabb.getVolume();

    for (int i = 0; i < 3; i++) {
        switch (m_alignment[i]) {
        case PivotAlignment::origin:
            pivot[i] = 0.f;
            break;
        case PivotAlignment::middle:
            pivot[i] = volume[i];
            break;
        case PivotAlignment::top:
            pivot[i] = aabb.m_max.y;
            break;
        case PivotAlignment::bottom:
            pivot[i] = aabb.m_min.y;
            break;
        case PivotAlignment::right:
            pivot[i] = aabb.m_min.x;
            break;
        case PivotAlignment::left:
            pivot[i] = aabb.m_max.x;
            break;
        }
    }

    pivot += m_offset;

    return pivot;
}
