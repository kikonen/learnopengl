#include "InputGeom.h"

#include <recastnavigation/Recast.h>

#include "mesh/VaoMesh.h"

namespace nav
{
    InputGeom::InputGeom(const mesh::Mesh* mesh)
        : m_mesh{ mesh }
    {
    }

    InputGeom::~InputGeom() {
        delete m_vertices;
        delete m_tris;
    }

    void InputGeom::build()
    {
        if (!m_dirty) return;

        // TODO KI this is wrong
        // => transform to translate, scale androtate mesh is required

        const auto* vaoMesh = dynamic_cast<const mesh::VaoMesh*>(m_mesh);
        if (!vaoMesh) return;

        m_vertexCount = m_mesh->getVertexCount();
        m_vertices = new float[m_vertexCount * 3];

        m_triCount = m_mesh->getIndexCount();
        m_tris = new int[m_triCount];

        {
            int i = 0;
            for (auto& vertex : vaoMesh->m_vertices)
            {
                const auto& pos = vertex.pos;
                m_vertices[i * 3 + 0] = pos.x;
                m_vertices[i * 3 + 1] = pos.y;
                m_vertices[i * 3 + 2] = pos.z;
                i++;
            }
        }

        {
            int i = 0;
            for (auto& index : vaoMesh->m_indeces)
            {
                m_tris[i] = index;
                i++;
            }
        }

        {
            float meshBMin[3];
            float meshBMax[3];
            rcCalcBounds(m_vertices, m_vertexCount, meshBMin, meshBMax);
            m_meshBMin = glm::vec3{ meshBMin[0], meshBMin[1], meshBMin[2] };
            m_meshBMax = glm::vec3{ meshBMax[0], meshBMax[1], meshBMax[2] };
        }

        m_dirty = false;
    }
}
