#pragma once

#include "asset/Assets.h"
#include "scene/RenderContext.h"
#include "scene/NodeRegistry.h"

class Renderer
{
public:
	Renderer(const Assets& assets);

	virtual void prepare();
	virtual void update(const RenderContext& ctx, NodeRegistry& registry);
	virtual void bind(const RenderContext& ctx);
	virtual void render(const RenderContext& ctx, NodeRegistry& registry);

protected:
	const Assets& assets;
};
