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

	if (!directional) {
		float lightMax = std::fmaxf(std::fmaxf(diffuse.r, diffuse.g), diffuse.b);
		radius = (-linear + std::sqrtf(linear * linear - 4 * quadratic * (constant - (256.0 / 5.0) * lightMax))) / (2 * quadratic);
	}
}

DirLightUBO Light::toDirLightUBO() {
	return { pos, use, dir, 0, ambient, diffuse, specular };
}

PointLightUBO Light::toPointightUBO()
{
	return {
		pos,
		use,

		ambient,
		diffuse,
		specular,

		constant,
		linear,
		quadratic,
		radius,
	};
}

SpotLightUBO Light::toSpotLightUBO()
{
	return {
		pos,
		use,
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
		radius,

		0,
		0,
	};
}
