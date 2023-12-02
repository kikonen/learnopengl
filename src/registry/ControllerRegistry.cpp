#include "ControllerRegistry.h"

#include "engine/UpdateContext.h"

#include "controller/NodeController.h"
#include "registry/Registry.h"

class Node;

ControllerRegistry::~ControllerRegistry()
{
    for (const auto& it : m_controllers) {
        for (auto* controller : it.second) {
            delete controller;
        }
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

        bool changed = false;
        for (auto* controller : it.second) {
            changed |= controller->update(ctx, *node);
        }

        if (changed) {
            // TODO KI *ALL* children must be updated also
            node->updateModelMatrix();
        }
    }
}

void ControllerRegistry::addController(
    ki::object_id targetId,
    NodeController* controller)
{
    if (!controller) return;

    Node* node = m_registry->m_nodeRegistry->getNode(targetId);

    if (!node) {
        KI_WARN(fmt::format("ADD_CONTROLLER: MISSING_NODE - targetId={}", targetId));
        return;
    }

    controller->prepare(m_assets, m_registry, *node);

    {
        const auto& it = m_controllers.find(targetId);
        if (it == m_controllers.end()) {
            m_controllers.insert(std::make_pair(targetId, std::vector<NodeController*>{}));
        }
    }

    {
        const auto& it = m_controllers.find(targetId);
        it->second.emplace_back(controller);
    }
}
