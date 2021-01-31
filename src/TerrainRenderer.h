#pragma once

#include <vector>

#include "Assets.h"
#include "RenderContext.h"
#include "Terrain.h"

class TerrainRenderer
{
public:
	TerrainRenderer(const Assets& assets);
	~TerrainRenderer();

	void prepare();
	void render(RenderContext& ctx, std::vector<Terrain*>& terrains);

public:
	const Assets& assets;
};

