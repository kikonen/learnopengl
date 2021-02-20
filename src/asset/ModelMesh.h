#pragma once

#include <string>
#include <array>
#include <vector>
#include <map>

#include "Assets.h"
#include "Shader.h"
#include "scene/RenderContext.h"

#include "Tri.h"
#include "Material.h"
#include "Shader.h"
#include "Vertex.h"
#include "Assets.h"
#include "Mesh.h"

class ModelMesh final : public Mesh {
public:
	ModelMesh(
		const std::string& modelName);

	ModelMesh(
		const std::string& modelName,
		const std::string& path);

	~ModelMesh();

	void prepare(const Assets& assets) override;
	void prepareBuffers(MeshBuffers& curr) override;
	void bind(const RenderContext& ctx, Shader* shader) override;
	void draw(const RenderContext& ctx) override;
	void drawInstanced(const RenderContext& ctx, int instanceCount) override;

public:
	Material* defaultMaterial = nullptr;

	std::vector<glm::uvec3> tris;
	std::vector<Vertex*> vertices;

	std::vector<Material*> materials;

private:
	const std::string modelName;
	const std::string path;

	MaterialsUBO materialsUbo;
	unsigned int  materialsUboId = -1;
	unsigned int materialsUboSize = -1;
};
