#include <string>

#include "Light.h"


Light::Light()
{
}

Light::~Light()
{
}

void Light::bind(Shader* shader)
{
	std::string lightPosName = { "lightPos" };
	shader->setVec3(lightPosName, pos);

	std::string lightColorName = { "lightColor" };
	shader->setVec3(lightColorName, color);
}

