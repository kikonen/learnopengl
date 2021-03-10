#pragma once

#include <glm/glm.hpp>

// https://stackoverflow.com/questions/49798189/glbuffersubdata-offsets-for-structs


struct UBO {
	unsigned int matrices;
	unsigned int data;
	unsigned int clipPlanes;
	unsigned int lights;

	unsigned int matricesSize;
	unsigned int dataSize;
	unsigned int clipPlanesSize;
	unsigned int lightsSize;
};

const unsigned int UBO_MATRICES = 0;
const unsigned int UBO_DATA = 1;
const unsigned int UBO_CLIP_PLANES = 2;
const unsigned int UBO_LIGHTS = 3;
const unsigned int UBO_MATERIALS = 4;
const unsigned int UBO_MATERIAL = 5;

const unsigned int MATERIAL_COUNT = 4;
const unsigned int LIGHT_COUNT = 8;
const unsigned int TEXTURE_COUNT = 8;
const unsigned int CLIP_PLANE_COUNT = 2;


// NOTE KI align 16
struct MatricesUBO {
	glm::mat4 projected;
	glm::mat4 projection;
	glm::mat4 view;
	glm::mat4 lightSpace;
};

// NOTE KI align 16
struct DataUBO {
	glm::vec3 viewPos;
	float time;

	glm::vec4 fogColor;
	float fogStart;
	float fogEnd;

	int pad1;
	int pad2;
};

// NOTE KI align 16
struct DirLightUBO {
	glm::vec3 pos;
	unsigned int use;
	glm::vec3 dir;
	int pad1;
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;
};

// NOTE KI align 16
struct PointLightUBO {
	glm::vec3 pos;
	unsigned int use;

	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;

	float constant;
	float linear;
	float quadratic;
	float radius;
};

// NOTE KI align 16
struct SpotLightUBO {
	glm::vec3 pos;
	unsigned int use;
	glm::vec3 dir;
	int pad1;
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;

	float constant;
	float linear;
	float quadratic;

	float cutoff;
	float outerCutoff;
	float radius;

	int pad2;
	int pad3;
};

// NOTE KI align 16
struct LightsUBO {
	DirLightUBO light;
	PointLightUBO pointLights[LIGHT_COUNT];
	SpotLightUBO spotLights[LIGHT_COUNT];
};

// NOTE KI align 16
struct MaterialUBO {
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 emission;
	glm::vec4 specular;
	float shininess;

	int diffuseTex;
	int emissionTex;
	int specularTex;
	int normalMap;
	int dudvMap;

	int pattern;

	float reflection;
	float refraction;

	float fogRatio;
	float tiling;

	int pad1;
};

// NOTE KI align 16
struct MaterialsUBO {
	MaterialUBO materials[MATERIAL_COUNT];
};

// NOTE KI align 16
struct ClipPlaneUBO {
	glm::vec4 plane;
	bool enabled;

	int pad1;
	int pad2;
	int pad3;
};

// NOTE KI align 16
struct ClipPlanesUBO {
	ClipPlaneUBO clipping[CLIP_PLANE_COUNT];
};
