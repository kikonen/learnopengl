#pragma once

#include "Updater.h"

class Node;

class SceneUpdater : public Updater
{
public:
    SceneUpdater(
        std::shared_ptr<Registry> registry,
        std::shared_ptr<std::atomic<bool>> alive);

    virtual void shutdown() override;
    virtual void prepare() override;

    void update(const UpdateContext& ctx);

    virtual std::string getStats() override;

private:
    void handleNodeAdded(Node* node);
};
