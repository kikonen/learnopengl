#pragma once

#include <string>

#include "Texture.h"
#include "Image.h"

class ImageTexture : public Texture
{
public:
	static ImageTexture* getTexture(const std::string& path, const TextureSpec& spec);

	ImageTexture(const std::string& path, const TextureSpec& spec);
	~ImageTexture();

	void prepare() override;

private:
	int load();

private:
	bool prepared = false;

	Image* image = nullptr;
};

