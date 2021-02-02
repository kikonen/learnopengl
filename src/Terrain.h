#pragma once

#include "glm/glm.hpp"
#include "Material.h"
#include "Shader.h"
#include "ModelMesh.h"
#include "RenderContext.h"


class Terrain
{
public:
	Terrain(int worldX, int worldZ, Material* material, Shader* shader);
	~Terrain();

	void prepare();
	void bind(RenderContext& ctx);
	void draw(RenderContext& ctx);
public:
	const int worldX;
	const int worldZ;
	Material* material;
	Shader* shader;

	glm::vec3 pos = { 0, 0, 0 };
private:
	ModelMesh* mesh = nullptr;
};

