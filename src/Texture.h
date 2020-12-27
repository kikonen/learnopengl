#pragma once

#include <string>
#include "Shader.h"

/*
* https://learnopengl.com/Getting-started/Textures
*/
class Texture
{
public:
	Texture(std::string& path);
	~Texture();

	void prepare(Shader* shader);
	void bind();
	int load();
public:
	std::string path;
	unsigned int id;

	unsigned char* image;
	int width, height, channels;
};

