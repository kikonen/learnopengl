#pragma once

#include "Updater.h"

class ParticleUpdater : public Updater
{
public:
    ParticleUpdater(
        Engine& engine);

    virtual void update(const UpdateContext& ctx) override;

    virtual std::string getStats() override;
};
