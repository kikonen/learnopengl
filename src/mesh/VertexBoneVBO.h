#pragma once

#include <glm/glm.hpp>

#include "animation/VertexBone.h"
#include "mesh/BoneEntry.h"

#include "VBO.h"

namespace mesh {
    class VertexBoneVBO : public VBO<animation::VertexBone, BoneEntry> {
    public:
        VertexBoneVBO(
            std::string_view name,
            int boneIdAttr,
            int weightAttr,
            int binding);

        virtual BoneEntry convertVertex(
            const animation::VertexBone& bone) override;

        virtual void prepareVAO(kigl::GLVertexArray& vao) override;

    private:
        const int m_weightAttr;

    };
}
