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
	if (!dirty) return;

	dir = glm::normalize(target - pos);

	if (!directional) {
		float lightMax = std::fmaxf(std::fmaxf(diffuse.r, diffuse.g), diffuse.b);
		radius = (-linear + std::sqrtf(linear * linear - 4 * quadratic * (constant - (256.0 / 5.0) * lightMax))) / (2 * quadratic);
	}
}

const glm::vec3& Light::getPos()
{
	return pos;
}

void Light::setPos(const glm::vec3& pos)
{
	this->pos = pos;
	dirty = true;
}

const glm::vec3& Light::getDir()
{
	return dir;
}

void Light::setDir(const glm::vec3& pos)
{
	this->dir = dir;
	dirty = true;
}

const glm::vec3& Light::getTarget()
{
	return target;
}

void Light::setTarget(const glm::vec3& target)
{
	this->target = target;
	dirty = true;
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
