#pragma once

#include "RenderContext.h"

class Raycast
{
public:
	Raycast();
	~Raycast();

	int castRay(const RenderContext& ctx);
};

