#pragma once

#include <string>

#include "ModelMesh.h"
#include "Assets.h"


class ModelMeshLoader
{
public:
	ModelMeshLoader(
		const Assets& assets,
		ModelMesh& mesh, 
		const std::string& path, 
		const std::string& modelName);
	~ModelMeshLoader();

	int load(
		std::vector<Tri>& tris,
		std::vector<Vertex>& vertexes,
		std::map<std::string, Material*>& materials
	);

public:
	const Assets& assets;
	ModelMesh& mesh;
	const std::string& path;
	const std::string& modelName;

	bool debugColors = false;
	Material* defaultMaterial = nullptr;
	bool overrideMaterials = true;

 	unsigned int textureCount = 0;

private:
	std::vector<glm::vec4> colors = {
		{ 1.0f, 0.0f, 0.0f, 1.f },
		{ 0.0f, 1.0f, 0.0f, 1.f },
		{ 0.0f, 0.0f, 1.0f, 1.f },

		{ 1.0f, 1.0f, 0.0f, 1.f },
		{ 0.0f, 1.0f, 1.0f, 1.f },
		{ 1.0f, 0.0f, 1.0f, 1.f },

		{ 1.0f, 0.5f, 0.5f, 1.f },
		{ 0.5f, 1.0f, 0.5f, 1.f },
		{ 0.5f, 0.5f, 1.0f, 1.f },
	};


	int resolveVertexIndex(
		std::vector<Vertex>& vertexes,
		std::vector<glm::vec3>& positions,
		std::vector<glm::vec2>& textures,
		std::vector<glm::vec3>& normals,
		Material* material,
		int pi,
		int ti,
		int ni);

	glm::vec3 createNormal(
		std::vector<glm::vec3>& positions,
		std::vector<glm::vec3>& normals,
		glm::uvec3 pi);

	void splitFragmentValue(const std::string& v, std::vector<std::string>& vv);
	int loadMaterials(std::map<std::string, Material*>& materials, std::string libraryName);
	std::string resolveTexturePath(const std::string& line);
};

