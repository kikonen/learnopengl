#pragma once

#include <string>

class Texture
{
public:
	Texture(std::string& path);
	~Texture();

	int load();
public:
	std::string path;
	unsigned char* image;
};

