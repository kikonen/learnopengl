#pragma once

#include <vector>

#include "Renderer.h"

class TerrainRenderer final : public Renderer
{
public:
	TerrainRenderer(const Assets& assets);
	~TerrainRenderer();

	void prepare() override;

	void update(const RenderContext& ctx, NodeRegistry& registry) override;

	void render(const RenderContext& ctx, NodeRegistry& registry) override;
};

