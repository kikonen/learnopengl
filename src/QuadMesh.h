#pragma once

#include "MeshBuffers.h"
#include "Assets.h"
#include "RenderContext.h"
#include "Shader.h"
#include "Material.h"
#include "Mesh.h"

class QuadMesh : public Mesh
{
public:
	QuadMesh();
	~QuadMesh();

	void prepare(const Assets& assets) override;
	void prepareBuffers(MeshBuffers& curr) override;
	void bind(const RenderContext& ctx, Shader* shader) override;
	void draw(const RenderContext& ctx) override;
	void drawInstanced(const RenderContext& ctx, int instanceCount) override;

public:
	Material* material = nullptr;

private:
	MeshBuffers buffers;

	MaterialsUBO materialsUbo;
	unsigned int  materialsUboId = -1;
	unsigned int materialsUboSize = -1;
};
