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
	unsigned int VBO, VAO, EBO;

	Shader* shader;

private:
	std::string modelName;
	std::string name;

	std::vector<Tri> tris;
	std::vector<std::array<float, 3>> vertexes;
    std::vector<std::array<float, 2>> textureVertexes;
	std::vector<std::array<float, 3>> normals;

	std::map<std::string, Material*> materials;

    float pos[3];

	float elapsed = 0;
 };

