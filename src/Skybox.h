#pragma once

#include <string>
#include <vector>

#include "RenderContext.h"
#include "Texture.h"
#include "ShaderInfo.h"
#include "Assets.h"

class Skybox
{
public:
	Skybox(const Assets& assets, const std::string& name);
	~Skybox();

	int prepare();

	void bind(const RenderContext& ctx);
	int draw(const RenderContext& ctx);

private:
	const Assets& assets;
	const std::string name;

	unsigned int textureID;
	unsigned int VBO = 0;
	unsigned int VAO = 0;
	unsigned int EBO = 0;

	Shader* shader;

	unsigned int loadCubemap(std::vector<std::string> faces);
};

