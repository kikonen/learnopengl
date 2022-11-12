#pragma once

#include <vector>
#include <string>

#include "Texture.h"
#include "Image.h"

class ImageTexture final : public Texture
{
public:
    static ImageTexture* getTexture(const std::string& path, const TextureSpec& spec);
    static const std::pair<int, const std::vector<const ImageTexture*>&> getPreparedTextures();

    ImageTexture(const std::string& path, const TextureSpec& spec);
    virtual ~ImageTexture();

    void prepare(const Assets& assets) override;

    bool isValid() { return m_valid; }
private:
    void load();

private:
    bool m_valid = false;

    std::unique_ptr<Image> image;
};

