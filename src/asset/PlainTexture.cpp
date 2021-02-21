#include "PlainTexture.h"

#include <mutex>
#include <glad/glad.h>

PlainTexture::PlainTexture(const std::string& name, const TextureSpec& spec, int width, int height)
	: Texture(name, spec)
{
	this->width = width; 
	this->height = height;

	format = GL_RGB;
	internalFormat = GL_RGB;
}

PlainTexture::~PlainTexture()
{
}

void PlainTexture::prepare()
{
}
