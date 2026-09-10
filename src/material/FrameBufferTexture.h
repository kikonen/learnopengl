#pragma once

#include <glm/glm.hpp>

#include "util/Ref.h"
#include "Texture.h"

namespace render
{
    class FrameBuffer;
}

// Single pixel, single FrameBuffer texture
class FrameBufferTexture : public Texture
{
public:
    FrameBufferTexture(
        std::string_view name,
        bool grayScale,
        bool gammaCorrect,
        material::TextureType type,
        const material::TextureSpec& spec,
        util::Ref<render::FrameBuffer> fbo,
        int attachmentIndex);

    ~FrameBufferTexture();

    void release() override;
    void prepareSingle() override;

    void prepareArray(
        ArrayTexture& arr,
        uint32_t layer) override;

    void updateSingle() override;

    void updateArray(
        ArrayTexture& arr,
        uint32_t layer) override;

private:
    util::Ref<render::FrameBuffer> m_frameBuffer;
    int m_attachmentIndex;

    GLuint m_samplerId{ 0 };
};
