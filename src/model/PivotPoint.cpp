#include "PivotPoint.h"

#include "mesh/MeshType.h"


glm::vec3 PivotPoint::resolve(mesh::MeshType* type) const
{
    const auto& volume = type->getAABB().getVolume();
    glm::vec3 pivot{ 0.f };

    for (int i = 0; i < 3; i++) {
        switch (alignment[i]) {
        case PivotAlignment::zero:
            pivot[i] = 0.f;
            break;
        }
    }

    //pivot = { 0.f, 2.f, 0.f };
    //pivot.y -= 36.f;
    return pivot;
}
