#pragma once

#include "RenderContext.h"

class Raycast final
{
public:
	Raycast();
	~Raycast();

	int castRay(const RenderContext& ctx);
};

