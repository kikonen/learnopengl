#pragma once

#include <string>

#include "ModelMesh.h"
#include "Assets.h"


class MeshLoader final
{
public:
	MeshLoader(
		const Assets& assets,
		const std::string& modelName);

	MeshLoader(
		const Assets& assets,
		const std::string& modelName,
		const std::string& path);

	~MeshLoader();

	ModelMesh* load();

private:
	int loadData(
		std::vector<glm::uvec3>& tris,
		std::vector<Vertex*>& vertices,
		std::vector<Material*>& materials
	);

public:
	const Assets& assets;
	const std::string modelName;
	const std::string path;

	Material* defaultMaterial = nullptr;
	bool overrideMaterials = false;
	bool loadTextures = true;

private:
	int resolveVertexIndex(
		std::map<glm::vec3*, Vertex*>& vertexMapping,
		std::vector<Vertex*>& vertices,
		std::vector<glm::vec3>& positions,
		std::vector<glm::vec2>& textures,
		std::vector<glm::vec3>& normals,
		std::vector<glm::vec3>& tangents,
		Material* material,
		int pi,
		int ti,
		int ni,
		int tangenti);

	glm::vec3 createNormal(
		std::vector<glm::vec3>& positions,
		std::vector<glm::vec3>& normals,
		glm::uvec3 pi);

	void createTangents(
		std::vector<glm::vec3>& positions,
		std::vector<glm::vec2>& textures,
		std::vector<glm::vec3>& normals,
		std::vector<glm::vec3>& tangents,
		const glm::uvec3& pi,
		const glm::uvec3& ti,
		const glm::uvec3& ni,
		glm::uvec3& tangenti);

	void splitFragmentValue(const std::string& v, std::vector<std::string>& vv);
	int loadMaterials(std::map<std::string, Material*>& materials, std::string libraryName);
	std::string resolveTexturePath(const std::string& line);
};

