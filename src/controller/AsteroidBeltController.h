#pragma once

#include "InstancedController.h"

class AsteroidBeltController : public InstancedController
{
public:
    AsteroidBeltController(const Assets& assets, Node* planet);

    void prepareInstanced(InstancedNode& node) override;
    bool updateInstanced(const RenderContext& ctx, InstancedNode& node) override;

private:
    Node* planet;
};

