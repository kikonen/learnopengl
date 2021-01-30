#pragma once

#include <vector>
#include <string>

#include "Light.h"
#include "RenderContext.h"
#include "Node.h"
#include "Skybox.h"
#include "ShadowMap.h"


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
	void drawNormals(RenderContext& ctx);
	void drawSelectedStencil(RenderContext& ctx);
	void drawNodes(RenderContext& ctx);
	void drawSelected(RenderContext& ctx);
	void drawBlended(std::vector<Node*>& nodes, RenderContext& ctx);

public:
	const Assets& assets;

	bool showNormals = false;
	Skybox* skybox = nullptr;
	ShadowMap* shadowMap = nullptr;

	std::vector<Node*> selection;
	std::vector<Node*> nodes;

	Light* dirLight = nullptr;
	std::vector<Light*> pointLights;
	std::vector<Light*> spotLights;

	Shader* stencilShader = nullptr;
	Shader* normalShader = nullptr;

private:
};
