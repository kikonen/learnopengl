#pragma once

#include <string>

#include "MeshBuffers.h"
#include "Assets.h"
#include "Shader.h"
#include "Material.h"
#include "Mesh.h"

#include "scene/RenderContext.h"

class QuadMesh final : public Mesh
{
public:
	QuadMesh(const std::string& name);
	virtual ~QuadMesh();

	bool hasReflection() override;
	bool hasRefraction() override;

	std::shared_ptr<Material> findMaterial(std::function<bool(Material&)> fn) override;
	void modifyMaterials(std::function<void(Material&)> fn) override;

	void prepare(const Assets& assets) override;
	void prepareBuffers(MeshBuffers& curr) override;
	void bind(const RenderContext& ctx, Shader* shader) override;
	void draw(const RenderContext& ctx) override;
	void drawInstanced(const RenderContext& ctx, int instanceCount) override;

public:
	std::string name;
	std::shared_ptr<Material> material = nullptr;
	std::vector<GLuint> textureIDs;

private:
	MaterialsUBO materialsUbo;
	unsigned int  materialsUboId = -1;
	unsigned int materialsUboSize = -1;
};
