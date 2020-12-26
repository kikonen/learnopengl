#pragma once

#include <string>
#include <vector>;

#include "Mesh.h"
#include "Tri.h"

class ModelMesh : Mesh
{
	ModelMesh(const std::string& modelPath);
	~ModelMesh();

	int load();
private:
	std::string modelPath;

	std::vector<Tri> tris;
	std::vector<float> vertexes;
    std::vector<float> textureVertexes;

    float pos[3];
 };

