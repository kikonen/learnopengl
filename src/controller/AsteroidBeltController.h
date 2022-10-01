#pragma once

#include <stduuid/uuid.h>

#include "InstancedController.h"

class AsteroidBeltController final : public InstancedController
{
public:
    AsteroidBeltController();

protected:
    void prepareInstanced(
        const Assets& assets,
        InstancedNode& node) override;

    bool updateInstanced(
        const RenderContext& ctx,
        InstancedNode& node,
        Node* parent) override;

private:
};

