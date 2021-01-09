#include "Texture.h"

#include <glad/glad.h>

#include "Shader.h"


Texture::Texture(const std::string& path)
	: path(path)
{
}

Texture::~Texture()
{
	delete image;
	image = NULL;
}

void Texture::prepare(Shader* shader)
{
	if (!image) {
		return;
	}

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

//	float borderColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
//	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->width, image->height, 0, GL_RGB, GL_UNSIGNED_BYTE, image->data);
//	glGenerateMipmap(GL_TEXTURE_2D);
}

void Texture::bind(Shader* shader)
{
	glActiveTexture(unitId);
	glBindTexture(GL_TEXTURE_2D, id);
	// NOTE KI other textures may caus mipmaps are discarded?!?
	glGenerateMipmap(GL_TEXTURE_2D);
}

int Texture::load() {
	Image* tmp = Image::getImage(path);
	int res = tmp->load();
	if (!res) {
		image = tmp;
	}
	return res;
}
