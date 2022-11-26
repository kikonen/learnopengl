#pragma once

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

class MeshBuffers final
{
public:
    MeshBuffers();
    ~MeshBuffers();

    const std::string str() const;

    void prepare(bool useVertex, bool useMaterial, bool useIndeces);

public:
    GLVertexArray VAO;
    GLBuffer VBO;
    GLBuffer VBO_MATERIAL;
    GLBuffer EBO;

private:
    bool m_prepared = false;
};
