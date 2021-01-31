#pragma once

#include <vector>
#include <string>

#include "Light.h"
#include "RenderContext.h"
#include "Node.h"
#include "Skybox.h"
#include "ShadowMap.h"
#include "NormalRenderer.h"

class Scene
{
public:
	Scene(const Assets& assets);
	~Scene();

	void prepare();
	void bind(RenderContext& ctx);
	void draw(RenderContext& ctx);

	void drawScene(RenderContext& ctx);

private:
	int drawNodes(RenderContext& ctx, bool selection);
	void drawBlended(std::vector<Node*>& nodes, RenderContext& ctx);

	void drawSelectionStencil(RenderContext& ctx);
public:
	const Assets& assets;

	bool showNormals = false;
	Skybox* skybox = nullptr;

	ShadowMapRenderer* shadowMapRenderer = nullptr;
	NormalRenderer* normalRenderer = nullptr;

	std::vector<Node*> nodes;

	Light* dirLight = nullptr;
	std::vector<Light*> pointLights;
	std::vector<Light*> spotLights;

	Shader* selectionShader = nullptr;

private:
};
