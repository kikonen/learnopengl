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

	void update(RenderContext& ctx, std::map<int, std::vector<Terrain*>>& typeTerrains);
	void render(RenderContext& ctx, std::map<int, std::vector<Terrain*>>& typeTerrains);

public:
	const Assets& assets;
};

