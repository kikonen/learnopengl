#pragma once

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

class MeshBuffers final
{
public:
    MeshBuffers();
    ~MeshBuffers();

    void prepare(bool useIndeces);

public:
    GLVertexArray VAO;
    GLBuffer VBO;
    GLBuffer EBO;

private:
    bool m_prepared = false;
};
