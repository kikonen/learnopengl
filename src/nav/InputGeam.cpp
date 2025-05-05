#include "InputGeom.h"

#include "mesh/Mesh.h"

namespace nav
{
    InputGeom::InputGeom(const mesh::Mesh* mesh)
        : m_mesh{ mesh }
    {
    }

    InputGeom::~InputGeom() = default;
}
