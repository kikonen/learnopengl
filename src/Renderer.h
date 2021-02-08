#pragma once

#include "Assets.h"
#include "RenderContext.h"
#include "Batch.h"

class Renderer
{
public:
	Renderer(const Assets& assets);

protected:
	const Assets& assets;
	Batch batch;
};

