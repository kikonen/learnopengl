#pragma once

#include "asset/MeshBuffers.h"

#include "Node.h"

class InstancedNode : public Node
{
public:
	InstancedNode(NodeType* type, NodeController* updater);
	~InstancedNode();

	void prepare(const Assets& assets) override;

	void updateBuffers(const RenderContext& ctx);

	bool update(const RenderContext& ctx) override;
	Shader* bind(const RenderContext& ctx, Shader* shader) override;
	void draw(const RenderContext& ctx) override;

private:

public:
	Batch modelBatch;
	Batch selectedBatch;

private:
	MeshBuffers selectedBuffers;

	bool buffersDirty = true;
};

