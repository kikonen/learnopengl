#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


Texture::Texture(std::string& path)
{
	this->path = path;
}

Texture::~Texture()
{
	stbi_image_free(image);
	image = NULL;
}

int Texture::load() {

	int width, height, channels;
//	stbi_set_flip_vertically_on_load(true);
	image = stbi_load(
		path.c_str(),
		&width,
		&height,
		&channels,
		STBI_rgb);

	return 0;
}
