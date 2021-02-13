#include "Image.h"

#include <iostream>
#include <map>
#include <mutex>

#include <stb_image.h>

static std::map<std::string, Image*> images;

static std::mutex images_lock;
static std::mutex load_lock;

Image* Image::getImage(const std::string& path)
{
	std::lock_guard<std::mutex> lock(images_lock);

	Image* image = images[path];
	if (!image) {
		image = new Image(path);
		images[path] = image;
	}
	return image;
}


Image::Image(const std::string& path)
	:path(path)
{
}

Image::~Image()
{
	stbi_image_free(data);
	data = nullptr;
}

// NOTE KI *NOT* thread safe
// https://github.com/nothings/stb/issues/309
int Image::load(bool flip) {
	if (loaded) {
		return res;
	}
	loaded = true;

	std::lock_guard<std::mutex> lock(load_lock);

	flipped = flip;
	stbi_set_flip_vertically_on_load(flip);

	data = stbi_load(
		path.c_str(),
		&width,
		&height,
		&channels,
		STBI_default);

	if (data) {
		std::cout << "LOADED::IMAGE " << path 
			<< " flipped=" << flipped
			<< " channels=" << channels 
			<< " width=" << width 
			<< " height=" << height << std::endl;
	}
	else {
		std::cout << "ERROR::IMAGE::LOAD_FAILED " << path << std::endl;
	}

	res = data ? 0 : -1;
	return res;
}
