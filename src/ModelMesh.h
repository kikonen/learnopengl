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

	int load();
public:

private:
	std::string modelName;
	std::string shaderName;

	std::string shaderPathBase;

	std::vector<Tri> tris;
	std::vector<Vertex> vertexes;

	std::map<std::string, Material*> materials;
	bool hasTexture = false;

	float elapsed = 0;
 };
