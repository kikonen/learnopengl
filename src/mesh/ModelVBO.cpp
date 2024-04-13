#include "ModelVBO.h"

#include "mesh/ModelMesh.h"


namespace mesh {
    ModelVBO::ModelVBO()
    {
    }

    ModelVBO::~ModelVBO()
    {
    }

    void ModelVBO::prepare(ModelMesh& mesh)
    {
        if (m_prepared) return;
        m_prepared = true;

        m_mesh = &mesh;

        m_indexCount = static_cast<uint32_t>(mesh.m_indeces.size() * 3);
    }

    void ModelVBO::clear()
    {
    }

    AABB ModelVBO::calculateAABB() const noexcept
    {
        //m_positions.calculateAABB();
        return {};
    }
}
