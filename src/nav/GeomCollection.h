#pragma once

#include <vector>
#include <memory>

#include <glm/glm.hpp>

#include "InputGeom.h"

namespace nav
{
    class InputGeom;

    class GeomCollection
    {
    public:
        GeomCollection();
        ~GeomCollection();

        void addInput(std::unique_ptr<nav::InputGeom> input);

        const std::vector<std::unique_ptr<InputGeom>>& getInputs() const
        {
            return m_inputs;
        }

        bool empty() const { return m_inputs.empty(); }
        bool dirty() const { return m_dirty; }

        void build();

        const glm::vec3& getNavMeshBoundsMin() const { return m_navMeshBMin; }
        const glm::vec3& getNavMeshBoundsMax() const { return m_navMeshBMax; }

        int getMaxTriCount() const { return m_maxTriCount; }

    private:
        bool m_dirty{ true };
        std::vector<std::unique_ptr<nav::InputGeom>> m_inputs;

        glm::vec3 m_navMeshBMin{ 0.f };
        glm::vec3 m_navMeshBMax{ 0.f };

        int m_totalTriCount{ 0 };
        int m_maxTriCount{ 0 };
    };
}
