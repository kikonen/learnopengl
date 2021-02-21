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
	if (prepared) return;
	prepared = true;

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, spec.mode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, spec.mode);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);

	glGenerateMipmap(GL_TEXTURE_2D);
}

void PlainTexture::setData(void* data, int size)
{
}

