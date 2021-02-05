#include "InstancedNode.h"
#include "InstancedUpdater.h";

InstancedNode::InstancedNode(Mesh* mesh, InstancedUpdater* updater)
	: Node(mesh),
	updater(updater)
{
}

InstancedNode::~InstancedNode()
{
}

void InstancedNode::prepareBuffer(std::vector<glm::mat4> matrices)
{
	glBufferData(GL_ARRAY_BUFFER, matrices.size() * sizeof(glm::mat4), &matrices[0], GL_DYNAMIC_DRAW);

	// NOTE mat4 as vertex attributes *REQUIRES* hacky looking approach
	std::size_t vec4Size = sizeof(glm::vec4);

	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
	glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(1 * vec4Size));
	glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
	glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));

	glVertexAttribDivisor(6, 1);
	glVertexAttribDivisor(7, 1);
	glVertexAttribDivisor(8, 1);
	glVertexAttribDivisor(9, 1);
}

void InstancedNode::updateBuffer(std::vector<glm::mat4> matrices)
{
	glBufferSubData(GL_ARRAY_BUFFER, 0, matrices.size() * sizeof(glm::mat4), &matrices[0]);
}

void InstancedNode::prepare(const Assets& assets)
{
	Node::prepare(assets);
	updater->prepare(*this);
	prepareBuffers();
	buffersDirty = false;
}

void InstancedNode::prepareBuffers()
{
	{
		glGenBuffers(1, &instanceBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
		glBindVertexArray(mesh->buffers.VAO);
		prepareBuffer(instanceMatrices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	{
		selectedBuffers.prepare();

		glGenBuffers(1, &selectedBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, selectedBuffer);
		glBindVertexArray(selectedBuffers.VAO);
		prepareBuffer(selectionMatrices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
}

void InstancedNode::updateBuffers(const RenderContext& ctx)
{
	{
		glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
		glBindVertexArray(mesh->buffers.VAO);
		updateBuffer(instanceMatrices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	{
		selectedBuffers.prepare();

		mesh->updateBuffers(selectedBuffers);

		glBindBuffer(GL_ARRAY_BUFFER, selectedBuffer);
		glBindVertexArray(selectedBuffers.VAO);
		updateBuffer(selectionMatrices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	buffersDirty = false;
}

void InstancedNode::update(const RenderContext& ctx)
{
	Node::update(ctx);
	bool updated = updater->update(ctx, *this);
	buffersDirty = buffersDirty || updated;
	if (buffersDirty) {
		updateBuffers(ctx);
	}
}

Shader* InstancedNode::bind(const RenderContext& ctx, Shader* shader)
{
	shader = Node::bind(ctx, shader);

	if (shader->selection) {
		glBindVertexArray(selectedBuffers.VAO);
	}

	glEnableVertexAttribArray(6);
	glEnableVertexAttribArray(7);
	glEnableVertexAttribArray(8);
	glEnableVertexAttribArray(9);

	return shader;
}

void InstancedNode::draw(const RenderContext& ctx)
{
	Shader* shader = mesh->bound;
	shader->drawInstanced.set(true);
	mesh->drawInstanced(ctx, shader->selection ? selectionMatrices.size() : instanceMatrices.size());
	glBindVertexArray(0);
}
