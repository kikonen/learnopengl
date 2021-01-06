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
	void bind(Shader* shader);
	int load();
public:
	const std::string path;

	unsigned int id;
	unsigned int unitId = 0;

	unsigned int textureIindex;

	unsigned char* image;
	int width, height, channels;
};

