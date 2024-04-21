#pragma once

#include "ModelVAO.h"

#include "VertexBoneVBO.h"

namespace mesh {
    class SkinnedModelVAO : public ModelVAO {
    public:
        SkinnedModelVAO(std::string_view name);
        ~SkinnedModelVAO();

        void clear();

        // @return VAO
        kigl::GLVertexArray* registerModel(
            mesh::ModelMesh* mesh);

        void updateRT();

    protected:
        void prepareVAO();

    public:
        VertexBoneVBO m_boneVbo;
    };
}
