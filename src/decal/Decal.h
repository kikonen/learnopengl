#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "pool/NodeHandle.h"

#include "ki/size.h"

#include "DecalSSBO.h"

namespace render
{
    class RenderContext;
}

struct UpdateContext;

namespace decal {
    // Decal is a bit similar to particle, but it's attached into
    // target node surface, and tracks it if target moves. Also
    // particle is perspective transformed (particle is just billboard)
    //
    // Like particle, decal allows animation based into spritesheet
    // define in material
    struct Decal {
        // local to target
        glm::vec3 m_position{ 0.f };
        // local to target
        glm::vec3 m_normal{ 0.f };

        pool::NodeHandle m_parent;

        // local rotation (radians) around normal axis
        float m_rotation{ 0.f };
        float m_scale{ 1.f };

        float m_lifetime{ 0.f };

        ki::material_index m_materialIndex{ 0 };

        float m_spriteSpeed{ 0.f };
        // floor used for render
        float  m_spriteActiveIndex{ 0.f };

        uint8_t m_spriteBaseIndex{ 0 };
        uint8_t m_spriteCount{ 1 };

        bool m_static : 1 { true };

        bool update(const UpdateContext& ctx) noexcept;

        void updateSSBO(DecalSSBO& ssbo) const noexcept;

        glm::mat4 getModelMatrix() const;
    };
}
