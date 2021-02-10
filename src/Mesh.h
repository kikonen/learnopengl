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

class Mesh final {
public:
	Mesh(
		const std::string& modelName);

	Mesh(
		const std::string& modelName,
		const std::string& path);

	~Mesh();

	void prepare(const Assets& assets);
	void updateBuffers(MeshBuffers& curr);
	void bind(const RenderContext& ctx, Shader* shader);
	void draw(const RenderContext& ctx);
	void drawInstanced(const RenderContext& ctx, int instanceCount);

public:
	Material* defaultMaterial = nullptr;

	MeshBuffers buffers;

	std::vector<Tri*> tris;
	std::vector<Vertex*> vertices;

	std::vector<Material*> materials;

private:
	const std::string modelName;
	const std::string path;

	MaterialsUBO materialsUbo;
	unsigned int  materialsUboId = -1;
	unsigned int materialsUboSize = -1;
};
