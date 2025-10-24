#pragma once

#include "TexturedVAO.h"

#include "VertexJointVBO.h"

namespace mesh {
    class SkinnedVAO : public TexturedVAO {
    public:
        SkinnedVAO(std::string_view name);
        ~SkinnedVAO();

        virtual void clear() override;

        void reserveVertexJoints(size_t count);

        void updateVertexJoints(
            uint32_t baseVbo,
            std::span<animation::VertexJoint> vertexJoints);

        virtual void updateRT() override;

    protected:
        virtual void prepareVAO() override;

    public:
        VertexJointVBO m_jointVbo;
    };
}
