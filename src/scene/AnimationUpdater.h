#pragma once

#include <memory>

#include "Updater.h"

class AnimationUpdater : public Updater
{
public:
    AnimationUpdater(
        std::shared_ptr<Registry> registry,
        std::shared_ptr<std::atomic<bool>> alive);

    virtual uint32_t getActiveCount() const noexcept override;
    virtual void update(const UpdateContext& ctx) override;
};
