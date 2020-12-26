#pragma once

#include <string>
#include <array>
#include <vector>
#include <map>

#include "Mesh.h"
#include "Tri.h"
#include "Material.h"


class ModelMesh : Mesh
{
public:
	ModelMesh(const std::string& modelName);
	~ModelMesh();

	int load();
	int loadMaterials(std::string libraryName);
private:
	std::string modelName;
	std::string name;

	std::vector<Tri> tris;
	std::vector<std::array<float, 3>> vertexes;
    std::vector<std::array<float, 3>> textureVertexes;
	std::vector<std::array<float, 3>> normals;

	std::map<std::string, Material*> materials;

    float pos[3];
 };

