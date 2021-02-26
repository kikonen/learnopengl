#pragma once

#include <string>

#include "MeshBuffers.h"
#include "Assets.h"
#include "Shader.h"
#include "Material.h"
#include "Mesh.h"

#include "scene/RenderContext.h"

class QuadMesh : public Mesh
{
public:
	QuadMesh(const std::string& name);
	~QuadMesh();

	bool hasReflection() override;
	bool hasRefraction() override;

	void setReflection(float reflection) override;
	void setRefraction(float refraction) override;

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
