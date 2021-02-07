#pragma once

#include <glm/glm.hpp>

#include "Node.h"

class Sprite : public Node
{
public:
	Sprite(int objectID, glm::vec2 size, Material* material);
	~Sprite();

	void prepare(const Assets& assets) override;

public:
	const glm::vec2 size;
	Material* material;
};

