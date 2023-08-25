#pragma once

#include <vector>
#include <string>
#include <future>

#include "Texture.h"

class Image;

class ImageTexture final : public Texture
{
public:
    static std::shared_future<ImageTexture*> getTexture(
        const std::string& name,
        const std::string& path,
        bool gammaCorrect,
        const TextureSpec& spec);

    static const std::pair<int, const std::vector<const ImageTexture*>&> getPreparedTextures();

    ImageTexture(
        const std::string& name,
        const std::string& path,
        bool gammaCorrect,
        const TextureSpec& spec);

    virtual ~ImageTexture();

    void prepare(
        const Assets& assets) override;

    bool isValid() { return m_valid; }

    void load();

private:
    const std::string m_path;

private:
    bool m_valid = false;

    std::unique_ptr<Image> m_image;
};

