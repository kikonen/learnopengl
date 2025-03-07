#pragma once

#include "Updater.h"

class ParticleUpdater : public Updater
{
public:
    ParticleUpdater(
        std::shared_ptr<Registry> registry,
        std::shared_ptr<std::atomic<bool>> alive);

    virtual void update(const UpdateContext& ctx) override;

    virtual std::string getStats() override;
};
