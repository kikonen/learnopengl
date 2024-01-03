#include "MeshBuffers.h"

#include <fmt/format.h>

#include "kigl/kigl.h"

namespace mesh {
    MeshBuffers::MeshBuffers()
    {
    }

    MeshBuffers::~MeshBuffers()
    {
    }

    const std::string MeshBuffers::str() const
    {
        return fmt::format(
            "<BUFFERS: vao={}, vbo={}, vbo_material={}, ebo={}>",
            (int)VAO, (int)VBO, (int)VBO_MATERIAL, (int)EBO);
    }

    void MeshBuffers::prepare(bool useVertex, bool useMaterial, bool useIndeces)
    {
        if (m_prepared) return;
        m_prepared = true;

        VAO.create("mesh");
        if (useVertex) {
            VBO.create();
        }
        if (useMaterial) {
            VBO_MATERIAL.create();
        }
        if (useIndeces) {
            EBO.create();
        }
    }
}
