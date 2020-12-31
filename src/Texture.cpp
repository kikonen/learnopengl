#include "Texture.h"

#include <glad/glad.h>
#include <stb_image.h>

#include "Shader.h"

Texture::Texture(std::string& path)
{
	this->path = path;
}

Texture::~Texture()
{
	stbi_image_free(image);
	image = NULL;
}

void Texture::prepare(Shader* shader)
{
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	float borderColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);

	std::string textureName = "texture1";
	shader->setInt(textureName, 0);
}

void Texture::bind()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, id);
}

int Texture::load() {

	stbi_set_flip_vertically_on_load(true);
	image = stbi_load(
		path.c_str(),
		&width,
		&height,
		&channels,
		STBI_rgb);

	return 0;
}
