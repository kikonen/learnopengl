#pragma once

#include <string>

#include "ModelMesh.h"

const std::string BASE_DIR = "3d_model";


class ModelMeshLoader
{
public:
	ModelMeshLoader(ModelMesh& mesh, const std::string& modelName);
	~ModelMeshLoader();

	int load(
		std::vector<Tri>& tris,
		std::vector<Vertex>& vertexes,
		std::map<std::string, Material*>& materials
	);

public:
	ModelMesh& mesh;
	const std::string& modelName;

	bool debugColors = false;
	Material* defaultMaterial = nullptr;
	bool overrideMaterials = true;

 	unsigned int textureCount = 0;

private:
	std::vector<glm::vec3> colors = {
		{ 1.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		{ 0.0f, 0.0f, 1.0f },

		{ 1.0f, 1.0f, 0.0f },
		{ 0.0f, 1.0f, 1.0f },
		{ 1.0f, 0.0f, 1.0f },

		{ 1.0f, 0.5f, 0.5f },
		{ 0.5f, 1.0f, 0.5f },
		{ 0.5f, 0.5f, 1.0f },
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

