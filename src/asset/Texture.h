#pragma once

#include <string>

#include <glad/glad.h>


struct TextureSpec {
	int mode = GL_CLAMP_TO_EDGE;
};

/*
* https://learnopengl.com/Getting-started/Textures
*/
class Texture 
{
public:
	Texture(const std::string& name, const TextureSpec& spec);
	~Texture();

	virtual void prepare() = 0;

public:
	const std::string name;
	const TextureSpec spec;

	unsigned int textureID = -1;

protected:
	bool prepared = false;

	int width = 0;
	int height = 0;
	int format = 0;
	int internalFormat = 0;
};

