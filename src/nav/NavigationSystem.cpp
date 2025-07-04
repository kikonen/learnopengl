#include "NavigationSystem.h"

#include "util/thread.h"

#include "physics/MeshGenerator.h"
#include "physics/PhysicsSystem.h"

#include "RecastContainer.h"
#include "Generator.h"
#include "Resolver.h"
#include "Path.h"
#include "NavigationMeshBuilder.h"

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
        m_generator{ std::make_shared<Generator>(m_container) },
        m_resolver{ std::make_unique<Resolver>(m_container) },
        m_builder{ std::make_unique<NavigationMeshBuilder>(m_generator) }
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
        m_builder->stop();
    }

    void NavigationSystem::prepare()
    {
    }

    void NavigationSystem::registerNode(pool::NodeHandle nodeHandle)
    {
        m_generator->registerNode(nodeHandle);
    }

    void NavigationSystem::unregisterNode(pool::NodeHandle nodeHandle)
    {
        m_generator->unregisterNode(nodeHandle);
    }

    void NavigationSystem::build()
    {
        setupPhysics();
        if (m_generator->isDirtyCollection()) {
            m_builder->start();
        }
    }

    void NavigationSystem::setupPhysics()
    {
        auto* physicsSystem = &physics::PhysicsSystem::get();
        if (!physicsSystem) return;

        if (m_physicsLevel == physicsSystem->getLevel()) return;
        m_physicsLevel = physicsSystem->getLevel();

        m_generator->clearMeshInstances();

        if (!m_physicsMeshGenerator) {
            m_physicsMeshGenerator = std::make_unique<physics::MeshGenerator>(*physicsSystem);
        }

        m_physicsMeshes = m_physicsMeshGenerator->generateMeshes(true);
        if (m_physicsMeshes.get()) {
            for (const auto& meshInstance : *m_physicsMeshes) {
                m_generator->registerMeshInstance(meshInstance);
            }
        }
    }

    Path NavigationSystem::findPath(const Query& query)
    {
        if (m_builder->isRunning()) return {};
        if (!m_generator->isReady()) return {};

        return m_resolver->resolve(query);
    }
}
