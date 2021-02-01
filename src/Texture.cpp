#include "Texture.h"

#include <glad/glad.h>

#include "Shader.h"


std::map<std::string, Texture*> textures;
std::map<std::string, Texture*> normalMaps;

Texture* Texture::getTexture(const std::string& path, bool normalMap)
{
	std::map<std::string, Texture*>& cache = normalMap ? normalMaps : textures;

	Texture* tex = cache[path];
	if (!tex) {
		tex = new Texture(path, normalMap);
		cache[path] = tex;
	}
	return tex;
}

Texture::Texture(const std::string& path, bool normalMap)
	: path(path),
	normalMap(normalMap)
{
}

Texture::~Texture()
{
	delete image;
	image = NULL;
}

void Texture::prepare()
{
	if (prepared) {
		return;
	}
	prepared = true;

	if (!image) {
		return;
	}

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

//	float borderColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
//	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	if (image->channels == 4) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->width, image->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->data);
	} else {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->width, image->height, 0, GL_RGB, GL_UNSIGNED_BYTE, image->data);
	}
	if (!normalMap) {
		glGenerateMipmap(GL_TEXTURE_2D);
	}
}

void Texture::bind(Shader* shader)
{
	glActiveTexture(unitID);
	glBindTexture(GL_TEXTURE_2D, textureID);
}

int Texture::load() {
	Image* tmp = Image::getImage(path);
	int res = tmp->load(true);
	if (!res) {
		image = tmp;
	}
	return res;
}
