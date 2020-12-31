#include <string>

#include "Light.h"


Light::Light()
{

}

Light::~Light()
{
}

void Light::bind(Mesh* mesh)
{
	std::string lightPosName = { "lightPos" };
	mesh->shader->setFloat3(lightPosName, pos.x, pos.y, pos.z);

	std::string lightColorName = { "lightColor" };
	mesh->shader->setFloat3(lightColorName, color.x, color.y, color.z);

}
