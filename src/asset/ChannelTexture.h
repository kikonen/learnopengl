#pragma once

#include <string>
#include <vector>
#include <future>
#include <memory>

#include "Texture.h"

class ImageTexture;

// compose together set of images into channels of single texture
class ChannelTexture final : public Texture
{
public:
    static std::shared_future<ChannelTexture*> getTexture(
        const std::string& name,
        const std::vector<ImageTexture*>& sourceTextures,
        const glm::vec4& defaults,
        bool is16Bbit,
        const TextureSpec& spec);

    static const std::pair<int, const std::vector<const ChannelTexture*>&> getPreparedTextures();

    ChannelTexture(
        const std::string& name,
        const std::vector<ImageTexture*>& sourceTextures,
        const glm::vec4& defaults,
        bool is16Bbit,
        const TextureSpec& spec);

    virtual ~ChannelTexture();

    void prepare(
        const Assets& assets) override;

    bool isValid() { return m_valid; }

    void load();

private:
    const std::vector<ImageTexture*> m_sourceTextures;
    const glm::vec4 m_defaults;
    const bool m_is16Bbit;

    int m_width{ 0 };
    int m_height{ 0 };
    unsigned char* m_data{ nullptr };


    bool m_valid{ false };
};
