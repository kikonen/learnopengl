#include "MeshBuffers.h"

#include "glad/glad.h"


MeshBuffers::MeshBuffers()
{
}

MeshBuffers::~MeshBuffers()
{
    if (prepared) {
        //glDeleteBuffers(EBO);
        //glDeleteBuffers(VAO);
        //glDeleteBuffers(VBO);
    }
}

void MeshBuffers::prepare(bool useIndeces)
{
    if (prepared) return;
    prepared = true;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    if (useIndeces) {
        glGenBuffers(1, &EBO);
    }
}
