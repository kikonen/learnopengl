#pragma once

#include <memory>

#include "Updater.h"

class AnimationUpdater : public Updater
{
public:
    AnimationUpdater(
        Engine& engine);

    virtual void update(const UpdateContext& ctx) override;

    virtual std::string getStats() override;
};
