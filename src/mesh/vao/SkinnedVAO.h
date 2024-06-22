#pragma once

#include "TexturedVAO.h"

#include "VertexBoneVBO.h"

namespace mesh {
    class SkinnedVAO : public TexturedVAO {
    public:
        SkinnedVAO(std::string_view name);
        ~SkinnedVAO();

        virtual void clear() override;

        void reserveVertexBones(size_t count);

        void updateVertexBones(
            uint32_t baseVbo,
            std::span<animation::VertexBone> vertexBones);

        virtual void updateRT() override;

    protected:
        virtual void prepareVAO() override;

    public:
        VertexBoneVBO m_boneVbo;
    };
}
