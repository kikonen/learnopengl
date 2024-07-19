#pragma once

#include <string>
#include <vector>
#include <future>
#include <memory>

#include "Texture.h"
#include "ChannelPart.h"

class ImageTexture;

// compose together set of images into channels of single texture
class ChannelTexture final : public Texture
{
public:
    static std::shared_future<ChannelTexture*> getTexture(
        std::string_view name,
        const std::vector<ChannelPart>& parts,
        const std::vector<ImageTexture*>& sourceTextures,
        const glm::vec4& defaults,
        int channelCount,
        bool is16Bbit,
        const TextureSpec& spec);

    ChannelTexture(
        std::string_view name,
        const std::vector<ChannelPart>& parts,
        const std::vector<ImageTexture*>& sourceTextures,
        const glm::vec4& defaults,
        int channelCount,
        bool is16Bbit,
        const TextureSpec& spec);

    virtual ~ChannelTexture();

    void prepare() override;

    bool isValid() { return m_valid; }

    void load();

private:
    const std::vector<ChannelPart> m_parts;
    const std::vector<ImageTexture*> m_sourceTextures;

    const glm::vec4 m_defaults;
    const int m_channelCount;
    const bool m_is16Bbit;

    int m_width{ 0 };
    int m_height{ 0 };
    unsigned char* m_data{ nullptr };


    bool m_valid{ false };
};
