#pragma once

#include <glm/glm.hpp>

// https://stackoverflow.com/questions/49798189/glbuffersubdata-offsets-for-structs


struct UBO {
	unsigned int matrices;
	unsigned int data;
	unsigned int lights;
	unsigned int materials;

	unsigned int matricesSize;
	unsigned int dataSize;
	unsigned int lightsSize;
	unsigned int materialsSize;
};

const unsigned int UBO_MATRICES = 0;
const unsigned int UBO_DATA = 1;
const unsigned int UBO_LIGHTS = 2;
const unsigned int UBO_MATERIALS = 3;
const unsigned int UBO_MATERIAL = 4;

const unsigned int MATERIAL_COUNT = 4;
const unsigned int LIGHT_COUNT = 8;


struct MatricesUBO {
	glm::mat4 projection;
	glm::mat4 view;
	glm::mat4 lightSpace;
};

struct DataUBO {
	glm::vec3 viewPos;
	float time;
};

struct DirLightUBO {
	glm::vec3 pos;
	int pad1;
	glm::vec3 dir;
	int pad2;
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;

	unsigned int use;
	int pad3;
	int pad4;
	int pad5;
};

struct PointLightUBO {
	glm::vec3 pos;
	int pad1;
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;

	float constant;
	float linear;
	float quadratic;

	unsigned int use;
};

struct SpotLightUBO {
	glm::vec3 pos;
	int pad1;
	glm::vec3 dir;
	int pad2;
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;

	float constant;
	float linear;
	float quadratic;

	float cutoff;
	float outerCutoff;

	unsigned int use;

	int pad3;
	int pad4;
};

struct LightsUBO {
	DirLightUBO light;
	PointLightUBO pointLights[LIGHT_COUNT];
	SpotLightUBO spotLights[LIGHT_COUNT];
};

struct MaterialUBO {
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 emission;
	glm::vec4 specular;
	float shininess;

	unsigned int hasDiffuseTex;
	unsigned int hasEmissionTex;
	unsigned int hasSpecularTex;
	unsigned int hasNormalMap;

	int pad1;
	int pad2;
	int pad3;
};

struct MaterialsUBO {
	MaterialUBO materials[MATERIAL_COUNT];
};
