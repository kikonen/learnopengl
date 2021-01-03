#pragma once

#include <string>
#include <array>
#include <vector>
#include <map>

#include "Mesh.h"
#include "Tri.h"
#include "Material.h"
#include "Shader.h"
#include "Camera.h"
#include "Vertex.h"

class ModelMesh : public Mesh {
public:
	ModelMesh(const Engine& engine, const std::string& modelName, const std::string& shaderName);
	~ModelMesh();

	virtual int prepare() override;
	virtual int bind(float dt, const glm::mat4& vpMat) override;
	virtual int draw(float dt, const glm::mat4& vpMat) override;

	int resolveVertexIndex(
		std::vector<glm::vec3>& positions,
		std::vector<glm::vec2>& textures,
		std::vector<glm::vec3>& normals,
		Material* material,
		int pi,
		int ti,
		int ni);

	int load();
	void splitFragmentValue(const std::string& v, std::vector<std::string>& vv);
	int loadMaterials(std::string libraryName);

public:

private:
	std::string modelName;
	std::string shaderName;
	std::string shaderPathBase;

	glm::vec3 color = { 0.8f, 0.8f, 0.0f };
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

	std::vector<Tri> tris;
	std::vector<Vertex> vertexes;

	std::map<std::string, Material*> materials;

	bool hasTexture = false;
	float elapsed = 0;
 };
