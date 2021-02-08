#pragma once

#include "glm/glm.hpp"
#include "Material.h"
#include "Shader.h"
#include "Node.h"
#include "RenderContext.h"


class Terrain : public Node
{
public:
	Terrain(NodeType* type, int worldX, int worldZ, Material* material, Shader* shader);
	~Terrain();

	void prepare(const Assets& assets) override;
public:
	const int worldX;
	const int worldZ;
	Material* material;
	Shader* shader;
};

