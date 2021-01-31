#pragma once

#include <vector>
#include <string>

#include "Light.h"
#include "RenderContext.h"
#include "Node.h"
#include "SkyboxRenderer.h"
#include "ShadowMapRenderer.h"
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
	SkyboxRenderer* skyboxRenderer = nullptr;

	ShadowMapRenderer* shadowMapRenderer = nullptr;
	NormalRenderer* normalRenderer = nullptr;

	std::vector<Node*> nodes;

	Light* dirLight = nullptr;
	std::vector<Light*> pointLights;
	std::vector<Light*> spotLights;

	Shader* selectionShader = nullptr;

private:
};
