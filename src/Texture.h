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
	Texture(const std::string& path, int textureMode, bool normalMap);
	~Texture();

	void prepare();
	void bind(Shader* shader);
	int load();

	static Texture* getTexture(const std::string& path, int textureMode, bool normalMap);
public:
	const std::string path;
	const int textureMode;
	const bool normalMap;

	unsigned int textureID = -1;
	unsigned int unitID = -1;
	unsigned int textureIndex = -1;

	Image* image = nullptr;

private:
	bool prepared = false;
	bool loaded = false;
	int res = 0;
};

