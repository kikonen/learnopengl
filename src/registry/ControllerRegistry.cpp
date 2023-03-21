#include "ControllerRegistry.h"

#include "engine/UpdateContext.h"

#include "controller/NodeController.h"
#include "registry/Registry.h"

class Node;

ControllerRegistry::~ControllerRegistry()
{
    for (const auto& it : m_controllers) {
        delete it.second;
    }
    m_controllers.clear();
}

void ControllerRegistry::prepare(Registry* registry)
{
    m_registry = registry;

    registry->m_dispatcher->addListener(
        event::Type::controller_add,
        [this](const event::Event& e) {
            auto& action = e.body.control;
            addController(action.target, action.controller);
        });
}

void ControllerRegistry::update(const UpdateContext& ctx)
{
    for (const auto& it : m_controllers) {
        Node* node = m_registry->m_nodeRegistry->getNode(it.first);
        if (!node) continue;
        if (it.second->update(ctx, *node)) {
            // TODO KI *ALL* children must be updated also
            node->updateModelMatrix();
        }
    }
}

void ControllerRegistry::addController(
    int targetObjectID,
    NodeController* controller)
{
    Node* node = m_registry->m_nodeRegistry->getNode(targetObjectID);

    if (!node) return;

    controller->prepare(m_assets, m_registry, *node);

    m_controllers.insert(std::make_pair(targetObjectID, controller));
}
