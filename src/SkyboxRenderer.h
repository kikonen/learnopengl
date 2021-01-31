#pragma once

#include <string>
#include <vector>

#include "RenderContext.h"
#include "Texture.h"
#include "ShaderInfo.h"
#include "Assets.h"

class SkyboxRenderer
{
public:
	SkyboxRenderer(const Assets& assets, const std::string& name);
	~SkyboxRenderer();

	int prepare();

	void assign(Shader* shader);
	int draw(const RenderContext& ctx);

public:
	unsigned int textureID;

private:
	const Assets& assets;
	const std::string name;

	unsigned int VBO = 0;
	unsigned int VAO = 0;
	unsigned int EBO = 0;

	Shader* shader = nullptr;

	unsigned int loadCubemap(std::vector<std::string> faces);
};

