#pragma once

#include <string>
#include <vector>

#include "Renderer.h"
#include "RenderContext.h"
#include "Texture.h"
#include "Assets.h"
#include "MeshBuffers.h"


class SkyboxRenderer : public Renderer
{
public:
	SkyboxRenderer(const Assets& assets, const std::string& name);
	~SkyboxRenderer();

	int prepare();

	void assign(Shader* shader);

	void update(const RenderContext& ctx);
	void render(const RenderContext& ctx);

public:
	unsigned int textureID;

private:
	const std::string name;

	MeshBuffers buffers;

	Shader* shader = nullptr;

	unsigned int loadCubemap(std::vector<std::string> faces);
};

