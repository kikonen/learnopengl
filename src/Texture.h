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

	unsigned int id = -1;
	unsigned int unitId = -1;
	unsigned int textureIndex = -1;

	unsigned char* image;
	int width, height, channels;
};

