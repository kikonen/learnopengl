#pragma once

#include <vector>

#include "Renderer.h"
#include "RenderContext.h"
#include "NodeRegistry.h"

class NormalRenderer final : public Renderer
{
public:
	NormalRenderer(const Assets& assets);

	void prepare();

	void update(const RenderContext& ctx, NodeRegistry& registry);
	void bind(const RenderContext& ctx, NodeRegistry& registry);
	void render(const RenderContext& ctx, NodeRegistry& registry);

private:
	Shader* normalShader = nullptr;
};

