#pragma once

#include "Node.h"


class InstancedNode : public Node
{
public:
	InstancedNode(Mesh* mesh, NodeUpdater* updater);
	~InstancedNode();

	void prepare(const Assets& assets) override;

	void prepareBuffers();
	void updateBuffers(const RenderContext& ctx);

	bool update(const RenderContext& ctx) override;
	Shader* bind(const RenderContext& ctx, Shader* shader) override;
	void draw(const RenderContext& ctx) override;

private:
	void prepareBuffer(std::vector<glm::mat4> matrices);
	void updateBuffer(std::vector<glm::mat4> matrices);

public:
	std::vector<glm::mat4> instanceMatrices;
	std::vector<glm::mat4> selectionMatrices;

private:
	MeshBuffers selectedBuffers;

	bool buffersDirty = true;

	unsigned int instanceBuffer = -1;
	unsigned int selectedBuffer = -1;
};

