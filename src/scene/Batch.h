#pragma once

#include "scene/RenderContext.h"
#include "asset/Shader.h"

class NodeType;
class Node;

class Batch final
{
public:
	Batch();

	void prepare(NodeType* type);
	void update(unsigned int count);
	void bind(const RenderContext& ctx, Shader* shader);
	void draw(const RenderContext& ctx, Node* node, Shader* shader);
	void flush(const RenderContext& ctx, NodeType* type);

public:
	bool prepared = false;
	unsigned int size = 0;

	unsigned int modelBuffer = -1;
	unsigned int normalBuffer = -1;

	std::vector<glm::mat4> modelMatrices;
	std::vector<glm::mat3> normalMatrices;
};

