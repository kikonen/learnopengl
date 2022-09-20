#pragma once

#include "InstancedController.h"

class AsteroidBeltController final : public InstancedController
{
public:
    AsteroidBeltController(Node* planet);

    void prepareInstanced(const Assets& assets, InstancedNode& node) override;
    bool updateInstanced(const RenderContext& ctx, InstancedNode& node) override;

private:
    Node* planet;
};

