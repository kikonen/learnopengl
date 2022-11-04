#include "MeshBuffers.h"

#include "glad/glad.h"


MeshBuffers::MeshBuffers()
{
}

MeshBuffers::~MeshBuffers()
{
    if (m_prepared) {
        //glDeleteBuffers(EBO);
        //glDeleteBuffers(VAO);
        //glDeleteBuffers(VBO);
    }
}

void MeshBuffers::prepare(bool useIndeces)
{
    if (m_prepared) return;
    m_prepared = true;

    glCreateVertexArrays(1, &VAO);
    glCreateBuffers(1, &VBO);
    if (useIndeces) {
        glCreateBuffers(1, &EBO);
    }
}
