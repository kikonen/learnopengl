#include "ControllerRegistry.h"

#include "util/thread.h"

#include "engine/UpdateContext.h"

#include "event/Dispatcher.h"

#include "engine/PrepareContext.h"

#include "controller/NodeController.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"

class Node;

namespace {
    static ControllerRegistry s_registry;
}

ControllerRegistry& ControllerRegistry::get() noexcept
{
    return s_registry;
}

ControllerRegistry::ControllerRegistry()
{
}

ControllerRegistry::~ControllerRegistry()
{
}

void ControllerRegistry::clear()
{
    ASSERT_WT();

    m_controllers.clear();
}

void ControllerRegistry::shutdown()
{
    ASSERT_WT();

    clear();
}

void ControllerRegistry::prepare(Registry* registry)
{
    ASSERT_WT();

    m_registry = registry;

    registry->m_dispatcherWorker->addListener(
        event::Type::controller_add,
        [this](const event::Event& e) {
            auto& action = e.body.control;
            auto handle = pool::NodeHandle::toHandle(action.target);
            addController(handle, action.controller);
        });
}

void ControllerRegistry::updateWT(const UpdateContext& ctx)
{
    for (const auto& it : m_controllers) {
        auto* node = it.first.toNode();
        if (!node) continue;

        bool changed = false;
        for (auto* controller : it.second) {
            changed |= controller->updateWT(ctx, *node);
        }
    }
}

void ControllerRegistry::addController(
    pool::NodeHandle target,
    NodeController* controller)
{
    if (!controller) return;

    auto* node = target.toNode();

    if (!node) {
        KI_WARN(fmt::format("ADD_CONTROLLER: MISSING_NODE - targetId={}", target.str()));
        return;
    }

    PrepareContext ctx{ m_registry };
    controller->prepare(ctx, *node);

    {
        if (const auto& it = m_controllers.find(target);
            it == m_controllers.end())
        {
            m_controllers.insert({ target, std::vector<NodeController*>{} });
        }
    }

    {
        m_controllers.find(target)->second.emplace_back(controller);
    }
}
