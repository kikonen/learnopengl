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
