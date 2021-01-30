#pragma once

#include <vector>

#include "RenderContext.h"
#include "Node.h"

class ShadowMap
{
public:
	ShadowMap(const Assets& assets);

	void prepare();
	void bind(RenderContext& ctx);
	void draw(RenderContext& ctx, std::vector<Node*>& nodes);

	void drawNodes(RenderContext& ctx, std::vector<Node*>& nodes);
	void drawBlendedNodes(std::vector<Node*>& nodes, RenderContext& ctx);

	void drawDebug(RenderContext& ctx);

public:
	glm::vec3 lightPos;
	glm::vec3 lightDir;

	unsigned int shadowMap;

private:
	const Assets& assets;

	unsigned int shadowMapFBO;

	Shader* shadowShader = nullptr;
	Shader* shadowDebugShader = nullptr;

};

