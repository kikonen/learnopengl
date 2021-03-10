#pragma once

#include <glm/glm.hpp>

#include "asset/Shader.h"
#include "asset/UBO.h"

class RenderContext;

class Light final
{
public:
	Light();
	~Light();

	void update(RenderContext& ctx);

	const glm::vec3& getPos();
	void setPos(const glm::vec3& pos);

	const glm::vec3& getDir();
	void setDir(const glm::vec3& pos);

	const glm::vec3& getTarget();
	void setTarget(const glm::vec3& target);

	DirLightUBO toDirLightUBO();
	PointLightUBO toPointightUBO();
	SpotLightUBO toSpotLightUBO();

public:
	bool dirty = true;
	bool use = true;
	bool directional = false;
	bool point = false;
	bool spot = false;

	// http://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation
	float constant = 1.f;
	float linear = 0.f;
	float quadratic = 0.f;

	// degrees
	float cutoffAngle = 0.f;
	// degrees
	float outerCutoffAngle = 0.f;

	float radius = 0.f;

	glm::vec4 ambient = { 0.2f, 0.2f, 0.2f, 1.f };
	glm::vec4 diffuse = { 0.5f, 0.5f, 0.5f, 1.f };
	glm::vec4 specular = { 1.0f, 1.0f, 1.0f, 1.f };

private:
	glm::vec3 pos = { 0.0f, 3.0f, 0.f };
	// dir = FROM pos to TARGET
	glm::vec3 dir = { 0.0f, 0.0f, 0.f };
	glm::vec3 target = { 0.f, 0.f, 0.f };
};

