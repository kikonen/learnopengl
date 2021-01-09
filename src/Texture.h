#pragma once

#include <string>
#include "Shader.h"

#include "Image.h"

/*
* https://learnopengl.com/Getting-started/Textures
*/
class Texture
{
public:
	Texture(const std::string& path);
	~Texture();

	void prepare(Shader* shader);
	void bind(Shader* shader);
	int load();

public:
	const std::string path;

	unsigned int id = -1;
	unsigned int unitId = -1;
	unsigned int textureIndex = -1;

	Image* image = nullptr;

private:
	bool loaded = false;
	int res = 0;
};

