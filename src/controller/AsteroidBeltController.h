#pragma once

#include "InstancedController.h"

class AsteroidBeltController final : public InstancedController
{
public:
    AsteroidBeltController(const std::shared_ptr<Assets> assets, Node* planet);

    void prepareInstanced(InstancedNode& node) override;
    bool updateInstanced(const RenderContext& ctx, InstancedNode& node) override;

private:
    Node* planet;
};

