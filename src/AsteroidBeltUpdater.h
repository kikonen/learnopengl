#pragma once

#include "InstancedUpdater.h"

class AsteroidBeltUpdater : public InstancedUpdater
{
public:
	AsteroidBeltUpdater(const Assets& assets, Node* planet);

	void prepare(InstancedNode& node) override;
	bool update(const RenderContext& ctx, InstancedNode& node) override;

private:
	Node* planet;
};

