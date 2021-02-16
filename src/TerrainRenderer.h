#pragma once

#include <vector>

#include "Renderer.h"
#include "Assets.h"
#include "RenderContext.h"
#include "Terrain.h"
#include "NodeRegistry.h"

class TerrainRenderer final : public Renderer
{
public:
	TerrainRenderer(const Assets& assets);
	~TerrainRenderer();

	void prepare();

	void update(const RenderContext& ctx, NodeRegistry& registry);
	void render(const RenderContext& ctx, NodeRegistry& registry);
};

