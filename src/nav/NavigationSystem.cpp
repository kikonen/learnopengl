#include "NavigationSystem.h"

#include "util/thread.h"

#include "RecastContainer.h"
#include "Generator.h"
#include "Resolver.h"
#include "Path.h"

namespace {
    static nav::NavigationSystem* s_system{ nullptr };
}

namespace nav
{
    void NavigationSystem::init() noexcept
    {
        assert(!s_system);
        s_system = new NavigationSystem();
    }

    void NavigationSystem::release() noexcept
    {
        auto* s = s_system;
        s_system = nullptr;
        delete s;
    }

    NavigationSystem& NavigationSystem::get() noexcept
    {
        assert(s_system);
        return *s_system;
    }

    NavigationSystem::NavigationSystem()
        : m_container{ std::make_shared<RecastContainer>() },
        m_generator{ std::make_unique<Generator>(m_container) },
        m_resolver{ std::make_unique<Resolver>(m_container) }
    {}

    NavigationSystem::~NavigationSystem()
    {
        clear();
    }

    void NavigationSystem::clear()
    {
        m_resolver->clear();
        m_generator->clear();
        m_container->clear();
    }

    void NavigationSystem::prepare()
    { }

    void NavigationSystem::registerNode(pool::NodeHandle nodeHandle)
    {
        m_generator->registerNode(nodeHandle);
        m_dirty = true;
    }

    void NavigationSystem::build()
    {
        m_generator->build();
        m_dirty = false;
    }

    Path NavigationSystem::findPath(const Query& query)
    {
        return m_resolver->resolve(query);
    }
}
