#pragma once

#include <string>
#include <vector>

#include "RenderContext.h"
#include "Texture.h"
#include "ShaderInfo.h"

class Skybox
{
public:
	Skybox(const std::string& name);
	~Skybox();

	int prepare(const std::string& baseDir);
	int draw(const RenderContext& ctx);

private:
	std::string name;

	unsigned int textureID;
	unsigned int VBO = 0;
	unsigned int VAO = 0;
	unsigned int EBO = 0;

	Shader* shader;

	unsigned int loadCubemap(std::vector<std::string> faces);
};

