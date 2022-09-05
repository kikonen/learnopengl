#pragma once

#include "asset/Assets.h"
#include "scene/RenderContext.h"
#include "scene/NodeRegistry.h"

class Renderer
{
public:
	Renderer(const Assets& assets);
	virtual ~Renderer();

	virtual void prepare();
	virtual void update(const RenderContext& ctx, NodeRegistry& registry);
	virtual void bind(const RenderContext& ctx);
	virtual void render(const RenderContext& ctx, NodeRegistry& registry);

protected:
	bool stepRender();

protected:
	const Assets& assets;

	int drawIndex;
	int drawSkip;

	bool rendered;
};
