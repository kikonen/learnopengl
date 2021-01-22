#include <string>

#include <cstdio>

#include "Light.h"

Light::Light()
{
}

Light::~Light()
{
}

DirLightUBO Light::toDirLightUBO() {
	return { dir, 0, ambient, diffuse, specular, use, 0, 0, 0 };
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
