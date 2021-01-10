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

	shader->setFloat("light.constant", constant);
	shader->setFloat("light.linear", linear);
	shader->setFloat("light.quadratic", quadratic);

	shader->setFloat("light.cutOff", glm::cos(glm::radians(cutoffAngle)));
	shader->setFloat("light.outerCutOff", glm::cos(glm::radians(outerCutoffAngle)));

	shader->setBool("light.directional", directional);
	shader->setBool("light.point", point);
	shader->setBool("light.spot", spot);
}

