#pragma once

#include <vector>

#include "RenderContext.h"
#include "Node.h"

class NormalRenderer
{
public:
	NormalRenderer(const Assets& assets);

	void prepare();
	void bind(RenderContext& ctx, std::vector<Node*>& nodes);
	void render(RenderContext& ctx, std::vector<Node*>& nodes);

private:
	const Assets& assets;
	Shader* shader = nullptr;

};

