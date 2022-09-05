#pragma once

#include "Texture.h"

class PlainTexture final : public Texture
{
public:
	PlainTexture(const std::string& name, const TextureSpec& spec, int width, int height);
	virtual ~PlainTexture();

	void prepare() override;
	void setData(void* data, int size);
};
