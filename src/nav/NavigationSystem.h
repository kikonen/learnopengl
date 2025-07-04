#pragma once

#include <vector>
#include <memory>

#include <glm/glm.hpp>

#include "pool/NodeHandle.h"

namespace physics
{
    class MeshGenerator;
}

namespace mesh {
    struct MeshInstance;
}

namespace nav
{
    class RecastContainer;
    class NavigationMeshBuilder;
    class Generator;
    class Resolver;
    struct Query;
    struct Path;

    class NavigationSystem
    {
    public:
        static void init() noexcept;
        static void release() noexcept;
        static nav::NavigationSystem& get() noexcept;

        NavigationSystem();
        ~NavigationSystem();

        void clear();

        void prepare();

        void registerNode(pool::NodeHandle nodeHandle);
        void unregisterNode(pool::NodeHandle nodeHandle);

        // Build must be done after registering all meshes
        void build();

        nav::Path findPath(const nav::Query& query);

    private:
        void setupPhysics();

    private:
        std::shared_ptr<nav::RecastContainer> m_container;
        std::unique_ptr<nav::Resolver> m_resolver;

        std::shared_ptr<nav::Generator> m_generator;
        std::unique_ptr<nav::NavigationMeshBuilder> m_builder;

        std::unique_ptr<physics::MeshGenerator> m_physicsMeshGenerator;
        std::shared_ptr<std::vector<mesh::MeshInstance>> m_physicsMeshes;
        ki::level_id m_physicsLevel{ 0 };
    };
}
