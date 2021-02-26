#include "ImageTexture.h"

#include <map>
#include <mutex>


namespace {
	std::map<std::string, ImageTexture*> textures;

	std::mutex textures_lock;
}

ImageTexture* ImageTexture::getTexture(const std::string& path, const TextureSpec& spec)
{
	std::lock_guard<std::mutex> lock(textures_lock);

	std::string cacheKey = path + "_" + std::to_string(spec.mode);

	ImageTexture* tex = textures[path];
	if (!tex) {
		tex = new ImageTexture(path, spec);
		int res = tex->load();
		textures[path] = tex;
	}
	return tex;
}

ImageTexture::ImageTexture(const std::string& path, const TextureSpec& spec)
	: Texture(path, spec)
{
}

ImageTexture::~ImageTexture()
{
	delete image;
}

void ImageTexture::prepare()
{
	if (prepared) return;
	prepared = true;

	if (!image) return;

	if (image->channels == 4) {
		format = GL_RGBA;
		internalFormat = GL_RGBA8;
	}
	else {
		format = GL_RGB;
		internalFormat = GL_RGB8;
	}

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, spec.mode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, spec.mode);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//	float borderColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
	//	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	//glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image->width, image->height, 0, format, GL_UNSIGNED_BYTE, image->data);

	glTexStorage2D(GL_TEXTURE_2D, 1, internalFormat, image->width, image->height);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image->width, image->height, format, GL_UNSIGNED_BYTE, image->data);

	glGenerateMipmap(GL_TEXTURE_2D);

	delete image;
	image = nullptr;
}

int ImageTexture::load() {
	Image* tmp = new Image(name);
	int res = tmp->load(true);
	if (res) {
		delete tmp;
		tmp = nullptr;
	}
	image = tmp;
	return res;
}
