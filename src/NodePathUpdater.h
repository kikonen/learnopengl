#pragma once

#include "NodeUpdater.h"

class NodePathUpdater : public NodeUpdater
{
public:
	NodePathUpdater(const Assets& assets, int pathMode);
	~NodePathUpdater();

	bool update(const RenderContext& ctx, Node& node) override;

private:
	const int pathMode;

};

