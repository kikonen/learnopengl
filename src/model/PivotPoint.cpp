#include "PivotPoint.h"

#include "model/NodeType.h"


glm::vec3 PivotPoint::resolveAlignment(const NodeType* type) const
{
    glm::vec3 align{ 0.f };

    const auto& aabb = type->getAABB();
    const auto& volume = aabb.getVolume();

    for (int i = 0; i < 3; i++) {
        switch (m_alignment[i]) {
        case PivotAlignment::origin:
            align[i] = 0.f;
            break;
        case PivotAlignment::middle:
            align[i] = volume[i];
            break;
        case PivotAlignment::top:
            align[i] = aabb.m_max.y;
            break;
        case PivotAlignment::bottom:
            align[i] = aabb.m_min.y;
            break;
        case PivotAlignment::right:
            align[i] = aabb.m_min.x;
            break;
        case PivotAlignment::left:
            align[i] = aabb.m_max.x;
            break;
        }
    }

    return align;
}
