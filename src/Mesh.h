#pragma once

#include <string>
#include <array>
#include <vector>
#include <map>

#include "Assets.h"
#include "Shader.h"
#include "RenderContext.h"

#include "Tri.h"
#include "Material.h"
#include "Shader.h"
#include "Vertex.h"
#include "Assets.h"
#include "MeshBuffers.h"

class Mesh {
public:
	Mesh(
		const std::string& modelName);

	Mesh(
		const std::string& modelName,
		const std::string& path);

	~Mesh();

	void prepare();
	void prepareBuffers(MeshBuffers& curr);
	Shader* bind(const RenderContext& ctx, Shader* shader);
	void draw(const RenderContext& ctx);
	void drawInstanced(const RenderContext& ctx, int instanceCount);

public:
	Shader* bound = nullptr;

	Shader* defaultShader = nullptr;
	Material* defaultMaterial = nullptr;

	MeshBuffers buffers;

	std::vector<Tri*> tris;
	std::vector<Vertex*> vertices;

	std::map<std::string, Material*> materials;

private:
	const std::string modelName;
	const std::string path;

	MaterialsUBO materialsUbo;
	unsigned int  materialsUboId = -1;
	unsigned int materialsUboSize = -1;
};
