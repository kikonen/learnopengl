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

	void drawNormals(RenderContext& ctx);
	void drawSelectedStencil(RenderContext& ctx);
	void drawNodes(RenderContext& ctx);
	void drawSelected(RenderContext& ctx);
	void drawBlended(std::vector<Node*>& nodes, RenderContext& ctx);

public:
	bool showNormals = false;
	Skybox* skybox;

	std::vector<Node*> selection;
	std::vector<Node*> nodes;
	std::vector<Light*> pointLights;
	std::vector<Light*> spotLights;

	Shader* stencilShader;
	Shader* normalShader;
private:
};
