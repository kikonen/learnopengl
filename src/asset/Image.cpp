#include "Image.h"

#include <iostream>
#include <map>
#include <mutex>

#include <stb_image.h>

#include "util/Log.h"

namespace {
	std::map<std::string, Image*> images;

	std::mutex images_lock;
	std::mutex load_lock;
}

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
		KI_INFO_SB("IMAGE::LOADED " << path
			+ " flipped=" << std::to_string(flipped)
			+ " channels=" << std::to_string(channels)
			+ " width=" << std::to_string(width)
			+ " height=" << std::to_string(height));
	}
	else {
		KI_ERROR_SB("IMAGE::LOAD_FAILED " << path);
	}

	res = data ? 0 : -1;
	return res;
}
