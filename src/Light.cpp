#include <string>

#include <cstdio>
#include <map>
#include <string>

#include "Light.h"

struct Names {
	int index;
	std::string use;
	std::string pos;
	std::string dir;
	std::string ambient;
	std::string diffuse;
	std::string specular;
	std::string constant;
	std::string linear;
	std::string quadratic;
	std::string cutoff;
	std::string outerCutoff;
};

std::map<int, const Names*> pointNames;
std::map<int, const Names*> spotNames;

Names* createNames(const std::string arr, int index) {
	Names* names = new Names();

	char name[255];

	names->index = index;

	sprintf_s(name, "%s[%i].use", arr.c_str(), index);
	names->use = name;

	sprintf_s(name, "%s[%i].pos", arr.c_str(), index);
	names->pos = name;

	sprintf_s(name, "%s[%i].dir", arr.c_str(), index);
	names->dir = name;

	sprintf_s(name, "%s[%i].ambient", arr.c_str(), index);
	names->ambient = name;

	sprintf_s(name, "%s[%i].diffuse", arr.c_str(), index);
	names->diffuse = name;

	sprintf_s(name, "%s[%i].specular", arr.c_str(), index);
	names->specular = name;

	sprintf_s(name, "%s[%i].constant", arr.c_str(), index);
	names->constant = name;

	sprintf_s(name, "%s[%i].linear", arr.c_str(), index);
	names->linear = name;

	sprintf_s(name, "%s[%i].quadratic", arr.c_str(), index);
	names->quadratic = name;

	sprintf_s(name, "%s[%i].cutoff", arr.c_str(), index);
	names->cutoff = name;

	sprintf_s(name, "%s[%i].outerCutoff", arr.c_str(), index);
	names->outerCutoff = name;

	return names;
}

const Names* getPointNames(int index) {
	if (pointNames.count(index)) {
		return pointNames[index];
	}

	const Names* names = createNames("pointLights", index);
	pointNames[index] = names;

	return pointNames[index];
}

const Names* getSpotNames(int index) {
	if (spotNames.count(index)) {
		return spotNames[index];
	}

	const Names* names = createNames("spotLights", index);
	spotNames[index] = names;

	return spotNames[index];
}


Light::Light()
{
}

Light::~Light()
{
}

void Light::bind(Shader* shader, int index)
{
	if (ambient.w != 1.0f || diffuse.w != 1.0f || specular.w != 1.0f) {
		int x = 0;
	}

	if (directional) {
		bindDirectional(shader);
	} else if (spot) {
		bindSpot(shader, index);
	} else {
		bindPoint(shader, index);
	}
}

void Light::bindDirectional(Shader* shader)
{
	if (!use) {
		shader->setBool("light.use", false);
		return;
	}
	shader->setBool("light.use", true);

	shader->setVec3("light.dir", dir);
	shader->setVec4("light.ambient", ambient);
	shader->setVec4("light.diffuse", diffuse);
	shader->setVec4("light.specular", specular);
}

void Light::bindPoint(Shader* shader, int index)
{
	const Names* names = getPointNames(index);

	shader->setBool(names->use, use);

	shader->setVec3(names->pos, pos);

	shader->setVec4(names->ambient, ambient);
	shader->setVec4(names->diffuse, diffuse);
	shader->setVec4(names->specular, specular);

	shader->setFloat(names->constant, constant);
	shader->setFloat(names->linear, linear);
	shader->setFloat(names->quadratic, quadratic);
}

void Light::bindSpot(Shader* shader, int index)
{
	const Names* names = getSpotNames(index);

	shader->setBool(names->use, use);

	shader->setVec3(names->pos, pos);
	shader->setVec3(names->dir, dir);

	shader->setVec4(names->ambient, ambient);
	shader->setVec4(names->diffuse, diffuse);
	shader->setVec4(names->specular, specular);

	shader->setFloat(names->constant, constant);
	shader->setFloat(names->linear, linear);
	shader->setFloat(names->quadratic, quadratic);

	shader->setFloat(names->cutoff, glm::cos(glm::radians(cutoffAngle)));
	shader->setFloat(names->outerCutoff, glm::cos(glm::radians(outerCutoffAngle)));
}

