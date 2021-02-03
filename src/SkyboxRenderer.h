#pragma once

#include <string>
#include <vector>

#include "RenderContext.h"
#include "Texture.h"
#include "Assets.h"
#include "MeshBuffers.h"


class SkyboxRenderer
{
public:
	SkyboxRenderer(const Assets& assets, const std::string& name);
	~SkyboxRenderer();

	int prepare();

	void assign(Shader* shader);
	void render(const RenderContext& ctx);

public:
	unsigned int textureID;

private:
	const Assets& assets;
	const std::string name;

	MeshBuffers buffers;

	Shader* shader = nullptr;

	unsigned int loadCubemap(std::vector<std::string> faces);
};

