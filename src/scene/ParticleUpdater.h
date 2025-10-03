#pragma once

#include "Updater.h"

class ParticleUpdater : public Updater
{
public:
    ParticleUpdater(
        Engine& engine,
        std::shared_ptr<std::atomic<bool>> alive);

    virtual void update(const UpdateContext& ctx) override;

    virtual std::string getStats() override;
};
