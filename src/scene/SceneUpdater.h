#pragma once

#include "Updater.h"

namespace model
{
    class Node;
}

class SceneUpdater : public Updater
{
public:
    SceneUpdater(
        Engine& engine);

    virtual void shutdown() override;
    virtual void prepare() override;

    void update(const UpdateContext& ctx);

    virtual std::string getStats() override;

private:
    void handleNodeAdded(model::Node* node);
    void handleNodeRemoved(model::Node* node);

private:
    event::Listen m_listen_scene_loaded;
    event::Listen m_listen_scene_unload;
    event::Listen m_listen_node_added;
    event::Listen m_listen_node_removed;
    event::Listen m_listen_script_run;
};
