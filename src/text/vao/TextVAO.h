#pragma once

#include "mesh/vao/TexturedVAO.h"

#include "AtlasTextureVBO.h"

namespace text {
    class TextVAO : public mesh::TexturedVAO {
    public:
        TextVAO(std::string_view name);
        ~TextVAO();

        virtual void clear() override;

        virtual void updateRT() override;

        uint32_t reserveAtlasCoords(size_t count);

        void updateAtlasCoords(
            uint32_t baseVbo,
            std::span<glm::vec2> atlasCoords);

    protected:
        virtual void prepareVAO() override;

    public:
        text::AtlasTextureVBO m_atlaxTexVbo;
    };
}
