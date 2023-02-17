#pragma once

#include <vector>

#include "asset/Assets.h"

#include "Surface.h"

class RenderContext;

namespace physics {
    class PhysicsEngine {
    public:
        PhysicsEngine(const Assets& assets);

        void prepare();
        void update(const RenderContext& ctx);

        inline Surface* registerSurface(std::unique_ptr<Surface> surface) {
            m_surfaces.push_back(std::move(surface));
            return m_surfaces.back().get();
        }

        inline float getLevel(const glm::vec3& pos) {
            float min = 1000000000000;
            for (auto& surface : m_surfaces) {
                if (!surface->withinBounds(pos)) continue;

                float level = surface->getLevel(pos);
                if (level < min) min = level;
            }
            return min;
        }

    private:
        const Assets& m_assets;

        std::vector<std::unique_ptr<Surface>> m_surfaces;
    };

}
