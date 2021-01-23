#pragma once

#include <vector>
#include <string>

#include "Light.h"
#include "RenderContext.h"
#include "Node.h"
#include "Skybox.h"

class Scene
{
public:
	Scene();
	~Scene();

	void prepare();
	void draw(RenderContext& ctx);

	void drawScene(RenderContext& ctx);

private:
	void drawNormals(RenderContext& ctx);
	void drawSelectedStencil(RenderContext& ctx);
	void drawNodes(RenderContext& ctx);
	void drawSelected(RenderContext& ctx);
	void drawBlended(std::vector<Node*>& nodes, RenderContext& ctx);

	void prepareShadowMap();
	void drawShadowMap(RenderContext& ctx);
public:
	bool showNormals = false;
	Skybox* skybox;

	std::vector<Node*> selection;
	std::vector<Node*> nodes;

	Light* dirLight;
	std::vector<Light*> pointLights;
	std::vector<Light*> spotLights;

	Shader* stencilShader;
	Shader* normalShader;

	unsigned int depthMapFBO;
	unsigned int depthMap;

private:
};
