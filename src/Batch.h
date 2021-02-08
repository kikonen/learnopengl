#pragma once

#include "RenderContext.h"
#include "Shader.h"


class Batch
{
public:
	Batch();

	void prepare(int batchSize);
	void update(int count);
	void bind(const RenderContext& ctx, Shader* shader);

public:
	unsigned int size = 10000;
	unsigned int buffer = -1;

	std::vector<glm::mat4> matrices;
};

