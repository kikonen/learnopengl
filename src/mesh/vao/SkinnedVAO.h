#pragma once

#include "TexturedVAO.h"

#include "VertexBoneVBO.h"

namespace mesh {
    class SkinnedVAO : public TexturedVAO {
    public:
        SkinnedVAO(std::string_view name);
        ~SkinnedVAO();

        virtual void clear() override;

        // @return VAO
        virtual const kigl::GLVertexArray* registerMesh(
            mesh::ModelMesh* mesh) override;

        virtual void updateRT() override;

    protected:
        virtual void prepareVAO() override;

    public:
        VertexBoneVBO m_boneVbo;
    };
}
