#pragma once

#include "NodeUpdater.h"

class InstancedNode;

class InstancedUpdater : public NodeUpdater
{
public:
	InstancedUpdater(const Assets& assets);
	~InstancedUpdater();

	virtual void prepare(Node& node) override;
	virtual bool update(const RenderContext& ctx, Node& node) override;

	virtual void prepareInstanced(InstancedNode& node);
	virtual bool updateInstanced(const RenderContext& ctx, InstancedNode& node);

};

