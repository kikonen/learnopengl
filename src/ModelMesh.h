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

class ModelMesh : Mesh
{
public:
	ModelMesh(const std::string& modelName);
	~ModelMesh();

	int prepare();
	int bind(Camera& camera, float dt);
	int draw(Camera& camera, float dt);

	int load();
	void splitFragmentValue(const std::string& v, std::vector<std::string>& vv);
	int loadMaterials(std::string libraryName);

public:
	unsigned int VAO;
	unsigned int VBO;
	unsigned int EBO;

	Shader* shader;

private:
	std::string modelName;
	std::string name;

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

    float pos[3];

	float elapsed = 0;
 };
