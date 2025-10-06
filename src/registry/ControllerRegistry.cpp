#include "ControllerRegistry.h"

#include "util/thread.h"

#include "engine/UpdateContext.h"

#include "event/Dispatcher.h"

#include "engine/PrepareContext.h"

#include "controller/NodeController.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"

namespace model
{
    class Node;
}

namespace
{
    thread_local std::exception_ptr lastException = nullptr;

    static ControllerRegistry* s_registry{ nullptr };
}

void ControllerRegistry::init() noexcept
{
    assert(!s_registry);
    s_registry = new ControllerRegistry();
}

void ControllerRegistry::release() noexcept
{
    auto* s = s_registry;
    s_registry = nullptr;
    delete s;
}

ControllerRegistry& ControllerRegistry::get() noexcept
{
    assert(s_registry);
    return *s_registry;
}

ControllerRegistry::ControllerRegistry()
{
}

ControllerRegistry::~ControllerRegistry()
{
}

void ControllerRegistry::clear()
{
    //ASSERT_WT();

    m_controllers.clear();
}

void ControllerRegistry::shutdown()
{
    ASSERT_WT();

    clear();
}

void ControllerRegistry::prepare(Engine* engine)
{
    ASSERT_WT();

    m_engine = engine;
}

void ControllerRegistry::updateWT(const UpdateContext& ctx)
{
    for (const auto& it : m_controllers) {
        auto* node = it.first.toNode();
        if (!node) continue;

        bool changed = false;
        for (auto& controller : it.second) {
            changed |= controller->updateWT(ctx, *node);
        }
    }
}

void ControllerRegistry::addController(
    pool::NodeHandle target,
    std::unique_ptr<NodeController> controller)
{
    if (!controller) return;

    auto* node = target.toNode();

    if (!node) {
        KI_WARN(fmt::format("ADD_CONTROLLER: MISSING_NODE - targetId={}", target.str()));
        return;
    }

    PrepareContext ctx{ *m_engine };
    controller->prepare(ctx, *node);

    {
        if (const auto& it = m_controllers.find(target);
            it == m_controllers.end())
        {
            m_controllers.insert({ target, std::vector<std::unique_ptr<NodeController>>{} });
        }
    }

    {
        m_controllers.find(target)->second.push_back(std::move(controller));
    }
}
