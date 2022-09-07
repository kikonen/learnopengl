#pragma once

#include <string>

#include "Texture.h"
#include "Image.h"

class ImageTexture final : public Texture
{
public:
    static ImageTexture* getTexture(const std::string& path, const TextureSpec& spec);

    ImageTexture(const std::string& path, const TextureSpec& spec);
    virtual ~ImageTexture();

    void prepare() override;

    bool isValid() { return valid; }
private:
    void load();

private:
    bool valid = false;
    bool prepared = false;

    std::unique_ptr<Image> image;
};

