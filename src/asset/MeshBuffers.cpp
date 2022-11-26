#include "MeshBuffers.h"

#include "ki/GL.h"

MeshBuffers::MeshBuffers()
{
}

MeshBuffers::~MeshBuffers()
{
}

void MeshBuffers::prepare(bool useVertex, bool useMaterial, bool useIndeces)
{
    if (m_prepared) return;
    m_prepared = true;

    VAO.create();
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
