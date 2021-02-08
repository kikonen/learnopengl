#pragma once

#include <glm/glm.hpp>

#include "Node.h"
#include "Material.h"

class Sprite : public Node
{
public:
	Sprite(NodeType* type, glm::vec2 size, Material* material);
	~Sprite();

	void prepare(const Assets& assets) override;

public:
	const glm::vec2 size;
	Material* material;
};

