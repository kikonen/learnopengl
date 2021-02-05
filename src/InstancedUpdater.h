#pragma once

#include "InstancedNode.h"

class InstancedUpdater
{
public:
	InstancedUpdater(const Assets& assets);
	~InstancedUpdater();

	virtual void prepare(InstancedNode& node);
	virtual bool update(const RenderContext& ctx, InstancedNode& node);

protected:
	const Assets& assets;
};

