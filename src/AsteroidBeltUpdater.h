#pragma once

#include "InstancedUpdater.h"

class AsteroidBeltUpdater : public InstancedUpdater
{
public:
	AsteroidBeltUpdater(const Assets& assets, Node* planet);

	void prepareInstanced(InstancedNode& node) override;
	bool updateInstanced(const RenderContext& ctx, InstancedNode& node) override;

private:
	Node* planet;
};

