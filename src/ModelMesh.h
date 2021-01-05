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
#include "Light.h"

class ModelMesh : public Mesh {
public:
	ModelMesh(const Engine& engine, const std::string& modelName, const std::string& shaderName);
	~ModelMesh();

	virtual int prepare() override;
	virtual int bind(const RenderContext& ctx) override;
	virtual int draw(const RenderContext& ctx) override;

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
 };
