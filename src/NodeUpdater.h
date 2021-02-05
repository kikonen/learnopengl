#pragma once

#include "Node.h"

class NodeUpdater
{
public:
	NodeUpdater(const Assets& assets);
	~NodeUpdater();

	virtual void prepare(Node& node);
	virtual bool update(const RenderContext& ctx, Node& node);

protected:
	const Assets& assets;
};

