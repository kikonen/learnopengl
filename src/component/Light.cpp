#include "Light.h"

#include <string>
#include <cstdio>

#include "scene/RenderContext.h"

Light::Light()
{
}

Light::~Light()
{
}

void Light::update(RenderContext& ctx)
{
	dir = glm::normalize(target - pos);
}

DirLightUBO Light::toDirLightUBO() {
	return { pos, 0, dir, 0, ambient, diffuse, specular, use, 0, 0, 0 };
}

PointLightUBO Light::toPointightUBO()
{
	return {
		pos,
		0,
		ambient,
		diffuse,
		specular,

		constant,
		linear,
		quadratic,
		use
	};
}

SpotLightUBO Light::toSpotLightUBO()
{
	return {
		pos,
		0,
		dir,
		0,
		ambient,
		diffuse,
		specular,

		constant,
		linear,
		quadratic,

		cutoffAngle,
		outerCutoffAngle,

		use,

		0,
		0,
	};
}
