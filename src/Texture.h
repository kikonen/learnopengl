#pragma once

#include <string>

/*
* https://learnopengl.com/Getting-started/Textures
*/
class Texture
{
public:
	Texture(std::string& path);
	~Texture();

	void prepare();
	void bind();
	int load();
public:
	std::string path;
	unsigned int id;

	unsigned char* image;
	int width, height, channels;
};

