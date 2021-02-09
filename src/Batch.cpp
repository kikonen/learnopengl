#include "Batch.h"

#include "NodeType.h"
#include "Node.h"

Batch::Batch()
{
}

void Batch::prepare(NodeType* type)
{
	if (size == 0) return;
	if (prepared) return;
	prepared = true;

	matrices.reserve(size);
	glm::mat4 tmp(0.f);
	for (int i = 0; i < size; i++) {
		matrices.push_back(tmp);
	}

	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBindVertexArray(type->mesh->buffers.VAO);
	glBufferData(GL_ARRAY_BUFFER, size * sizeof(glm::mat4), &matrices[0], GL_DYNAMIC_DRAW);

	// NOTE mat4 as vertex attributes *REQUIRES* hacky looking approach
	std::size_t vec4Size = sizeof(glm::vec4);

	glVertexAttribPointer(ATTR_INSTANCE_MATRIX_1, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
	glVertexAttribPointer(ATTR_INSTANCE_MATRIX_2, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(1 * vec4Size));
	glVertexAttribPointer(ATTR_INSTANCE_MATRIX_3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
	glVertexAttribPointer(ATTR_INSTANCE_MATRIX_4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));

	glVertexAttribDivisor(ATTR_INSTANCE_MATRIX_1, 1);
	glVertexAttribDivisor(ATTR_INSTANCE_MATRIX_2, 1);
	glVertexAttribDivisor(ATTR_INSTANCE_MATRIX_3, 1);
	glVertexAttribDivisor(ATTR_INSTANCE_MATRIX_4, 1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Batch::update(int count)
{
	if (size == 0) return;

	if (count > size) {
		count = size;
	}

	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(glm::mat4), &matrices[0]);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Batch::bind(const RenderContext& ctx, Shader* shader)
{
	if (size == 0) return;

	shader->drawInstanced.set(true);

	glEnableVertexAttribArray(ATTR_INSTANCE_MATRIX_1);
	glEnableVertexAttribArray(ATTR_INSTANCE_MATRIX_2);
	glEnableVertexAttribArray(ATTR_INSTANCE_MATRIX_3);
	glEnableVertexAttribArray(ATTR_INSTANCE_MATRIX_4);

	matrices.clear();
}

void Batch::draw(const RenderContext& ctx, Node* node, Shader* shader)
{
	if (size == 0) {
		node->bind(ctx, shader);
		node->draw(ctx);
		return;
	}

	node->bindBatch(ctx, *this);

	if (matrices.size() < size) return;

	flush(ctx, node->type);
}

void Batch::flush(const RenderContext& ctx, NodeType* type)
{
	if (size == 0 || matrices.empty()) return;
	
	update(matrices.size());
	type->mesh->drawInstanced(ctx, matrices.size());
	matrices.clear();
}
