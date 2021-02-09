#pragma once

#include "RenderContext.h"
#include "Shader.h"

class NodeType;

class Batch final
{
public:
	Batch();

	void prepare(NodeType* type);
	void update(int count);
	void bind(const RenderContext& ctx, Shader* shader);

public:
	bool prepared = false;
	unsigned int size = 0;
	unsigned int buffer = -1;

	std::vector<glm::mat4> matrices;
};

