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
	shader->setVec3("light.pos", pos);
	shader->setVec3("light.dir", dir);
	shader->setVec3("light.ambient", ambient);
	shader->setVec3("light.diffuse", diffuse);
	shader->setVec3("light.specular", specular);

	shader->setBool("light.directional", directional);
}

