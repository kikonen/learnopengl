#pragma once

#include <glm/glm.hpp>

#include <ode/ode.h>

#include "size.h"
#include "asset/AABB.h"

#include "size.h"

class Image;
class Node;

namespace physics {
    struct Object;
    class MeshGenerator;

    class HeightMap {
        friend class physics::MeshGenerator;

    public:
        HeightMap();
        HeightMap(HeightMap&& o);

        ~HeightMap();

        inline bool isReady() const noexcept
        {
            return m_prepared;
        }

        void prepare(
            Image* image,
            bool flip);

        void create(
            dWorldID worldId,
            dSpaceID spaceId,
            physics::Object& object) const;

        const AABB& getAABB() const noexcept { return m_aabb; }
        void setAABB(const AABB& aabb) { m_aabb = aabb; }

        inline bool withinBounds(const glm::vec3 pos) const noexcept
        {
            return pos.x >= m_aabb.m_min.x &&
                pos.x <= m_aabb.m_max.x &&
                pos.y >= m_aabb.m_min.y &&
                pos.y <= m_aabb.m_max.y;
        }

        float getLevel(const glm::vec3& pos) const noexcept;

        // Using texture coordinates
        //
        // @param u [0, 1]
        // @param v [0, 1]
        float getTerrainHeight(float u, float v) const noexcept;

    public:
        physics::height_map_id m_id{ 0 };

        //const std::unique_ptr<Image> m_image;

        const Node* m_origin{ nullptr };

        int m_worldTileSize{ 0 };
        // X -dir
        int m_worldSizeU{ 0 };
        // Z -dir (depth)
        int m_worldSizeV{ 0 };

        glm::vec2 m_verticalRange{ 0.f, 32.f };
        float m_horizontalScale{ 1.f };

    private:
        bool m_prepared{ false };

        int m_minH{ 0 };
        int m_maxH{ 0 };

        float m_minY{ 0.f };
        float m_maxY{ 0.f };

        // X -dir (width) samples in m_heightData
        int m_dataWidth{ 0 };
        // Z -dir (depth) samples in m_heightData
        int m_dataDepth{ 0 };

        float* m_heightData{ nullptr };
        bool m_flipH{ false };

        AABB m_aabb{};
    };

}
