#pragma once

class MeshBuffers final
{
public:
    MeshBuffers();
    ~MeshBuffers();

    void prepare(bool useIndeces);
public:
    unsigned int VBO = 0;
    unsigned int VAO = 0;
    unsigned int EBO = 0;

private:
    bool m_prepared = false;
};
