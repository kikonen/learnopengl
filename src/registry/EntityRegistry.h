#pragma once

#include <bitset>
#include <vector>
#include <atomic>
#include <span>

#include "kigl/GLBuffer.h"
#include "kigl/GLFence.h"

#include "shader/SSBO.h"

#include "EntitySSBO.h"

namespace render
{
    class RenderContext;
}

struct UpdateContext;

//
// Manage SSBO buffer for registered entity instances
// - allow binding all matrices at *once*, and update them
//   once per frame instead of pushing them multiple times into dynamic VBOs
//
class EntityRegistry {
public:
    static void init() noexcept;
    static void release() noexcept;
    static EntityRegistry& get() noexcept;

    EntityRegistry();
    ~EntityRegistry();
    EntityRegistry& operator=(const EntityRegistry&) = delete;

    void clear();
    void prepare();

    void updateRT(const UpdateContext& ctx);
    void postRT(const UpdateContext& ctx);

private:
    kigl::GLBuffer m_ssbo{ "entity_ssbo" };
    kigl::GLFence m_fence{ "fence_entity" };
    bool m_useMapped{ false };
    bool m_useInvalidate{ false };
    bool m_useFence{ false };
    bool m_useFenceDebug{ false };
};
