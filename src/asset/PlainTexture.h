#pragma once

#include "Texture.h"

class PlainTexture final : public Texture
{
public:
	PlainTexture(const std::string& name, const TextureSpec& spec, int width, int height);
	~PlainTexture();

	void prepare() override;
};

