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
	void bind(RenderContext& ctx);
	void draw(RenderContext& ctx);

	void drawScene(RenderContext& ctx);

private:
	void drawNormals(RenderContext& ctx);
	void drawSelectedStencil(RenderContext& ctx);
	void drawNodes(RenderContext& ctx);
	void drawSelected(RenderContext& ctx);
	void drawBlended(std::vector<Node*>& nodes, RenderContext& ctx);

	void prepareShadowMap();
	void bindShadowMap(RenderContext& ctx);
	void drawShadowMap(RenderContext& ctx);

	void drawShadow(RenderContext& ctx);
	void drawBlendedShadow(std::vector<Node*>& nodes, RenderContext& ctx);

	void drawDebugShadowMap(RenderContext& ctx);
public:
	bool showNormals = false;
	Skybox* skybox = nullptr;

	//glm::vec3 groundOffset(0.f, 15.f, -15.f);
	glm::vec3 groundOffset = { 0.f, 15.f, -40.f };

	std::vector<Node*> selection;
	std::vector<Node*> nodes;

	Light* dirLight = nullptr;
	std::vector<Light*> pointLights;
	std::vector<Light*> spotLights;

	Shader* stencilShader = nullptr;
	Shader* normalShader = nullptr;

	Shader* shadowShader = nullptr;
	Shader* shadowDebugShader = nullptr;

	unsigned int shadowMapFBO;
	unsigned int shadowMap;

private:
};
