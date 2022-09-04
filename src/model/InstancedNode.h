#pragma once

#include "asset/MeshBuffers.h"

#include "Node.h"

class InstancedNode : public Node
{
public:
	InstancedNode(std::shared_ptr<NodeType> type, NodeController* updater);
	~InstancedNode();

	void prepare(const Assets& assets) override;

	void updateBuffers(const RenderContext& ctx);

	bool update(const RenderContext& ctx) override;
	void bind(const RenderContext& ctx, std::shared_ptr<Shader> shader) override;
	void draw(const RenderContext& ctx) override;

private:

public:
	Batch modelBatch;
	Batch selectedBatch;

private:
	MeshBuffers selectedBuffers;

	bool buffersDirty = true;
};

