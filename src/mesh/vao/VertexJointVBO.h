#pragma once

#include <glm/glm.hpp>

#include "animation/VertexJoint.h"

#include "JointEntry.h"

#include "VBO.h"

namespace mesh {
    class VertexJointVBO : public VBO<animation::VertexJoint, JointEntry> {
    public:
        VertexJointVBO(
            std::string_view name,
            int jointIdAttr,
            int weightAttr,
            int binding);

        virtual JointEntry convertVertex(
            const animation::VertexJoint& joint) override;

        virtual void prepareVAO(kigl::GLVertexArray& vao) override;

    private:
        const int m_weightAttr;

    };
}
