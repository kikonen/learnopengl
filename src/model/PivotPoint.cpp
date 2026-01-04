#include "PivotPoint.h"

#include "model/NodeType.h"


namespace model
{
    glm::vec3 PivotPoint::resolveAlignment(const model::NodeType* type) const
    {
        glm::vec3 align{ 0.f };

        const auto& aabb = type->getAABB();
        const auto& localVolume = aabb.toLocalVolume();
        const auto& min = aabb.getMin();
        const auto& max = aabb.getMax();

        for (int i = 0; i < 3; i++) {
            switch (m_alignment[i]) {
            case PivotAlignment::origin:
                align[i] = 0.f;
                break;
            case PivotAlignment::middle:
                align[i] = localVolume[i];
                break;
            case PivotAlignment::top:
                align[i] = max.y;
                break;
            case PivotAlignment::bottom:
                align[i] = min.y;
                break;
            case PivotAlignment::right:
                align[i] = min.x;
                break;
            case PivotAlignment::left:
                align[i] = max.x;
                break;
            }
        }

        return align;
    }
}
