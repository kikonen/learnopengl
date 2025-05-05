#pragma once

#include <memory>

#include <glm/glm.hpp>

namespace mesh
{
    class Mesh;
}

namespace nav
{
    class RecastContainer;
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
        void registerMesh(const mesh::Mesh* mesh);

        // Build must be done after registering all meshes
        void build();

        nav::Path findPath(const nav::Query& query);

    private:
        bool m_dirty{ true };

        std::shared_ptr<nav::RecastContainer> m_container;
        std::unique_ptr<nav::Resolver> m_resolver;
        std::unique_ptr<nav::Generator> m_generator;
    };
}
