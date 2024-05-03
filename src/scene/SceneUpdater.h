#pragma once

#include "Updater.h"

class Node;

class SceneUpdater : public Updater
{
public:
    SceneUpdater(
        std::shared_ptr<Registry> registry,
        std::shared_ptr<std::atomic<bool>> alive);

    virtual void prepare() override;

    virtual uint32_t getActiveCount() const noexcept override;
    void update(const UpdateContext& ctx);

private:
    void handleNodeAdded(Node* node);
};
