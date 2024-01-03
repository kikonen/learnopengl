#pragma once

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

namespace mesh {
    class MeshBuffers final
    {
    public:
        MeshBuffers();
        ~MeshBuffers();

        const std::string str() const;

        void prepare(bool useVertex, bool useMaterial, bool useIndeces);

    public:
        kigl::GLVertexArray VAO;
        kigl::GLBuffer VBO{ "meshVBO" };
        kigl::GLBuffer VBO_MATERIAL{ "meshMaterialVBO" };
        kigl::GLBuffer EBO{ "meshEBO" };

    private:
        bool m_prepared = false;
    };
}
