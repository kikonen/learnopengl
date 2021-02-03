#pragma once

#include "glm/glm.hpp"
#include "Material.h"
#include "Shader.h"
#include "Node.h"
#include "RenderContext.h"


class Terrain
{
public:
	Terrain(int worldX, int worldZ, Material* material, Shader* shader);
	~Terrain();

	void prepare();
	virtual ShaderInfo* bind(const RenderContext& ctx, Shader* shader);
	void draw(RenderContext& ctx);
public:
	const int worldX;
	const int worldZ;
	Material* material;
	Shader* shader;

	glm::vec3 pos = { 0, 0, 0 };
private:
	Node* node = nullptr;
	Mesh* mesh = nullptr;
};

