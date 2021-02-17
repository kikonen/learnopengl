#pragma once

#include <string>

#include "MeshBuffers.h"
#include "Assets.h"
#include "RenderContext.h"
#include "Shader.h"
#include "Material.h"
#include "Mesh.h"

class QuadMesh : public Mesh
{
public:
	QuadMesh(const std::string& name);
	~QuadMesh();

	void prepare(const Assets& assets) override;
	void prepareBuffers(MeshBuffers& curr) override;
	void bind(const RenderContext& ctx, Shader* shader) override;
	void draw(const RenderContext& ctx) override;
	void drawInstanced(const RenderContext& ctx, int instanceCount) override;

public:
	std::string name;
	Material* material = nullptr;

private:
	MaterialsUBO materialsUbo;
	unsigned int  materialsUboId = -1;
	unsigned int materialsUboSize = -1;
};
