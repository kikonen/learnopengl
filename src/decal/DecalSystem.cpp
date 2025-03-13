#include "DecalSystem.h"

#include "asset/Assets.h"

#include "util/thread.h"

#include "kigl/GLState.h"

#include "shader/SSBO.h"
#include "shader/ProgramRegistry.h"

#include "engine/UpdateContext.h"
#include "engine/PrepareContext.h"

#include "render/RenderContext.h"
#include "render/DebugContext.h"

#include "registry/Registry.h"

#include "Decal.h"
#include "DecalCollection.h"

namespace {
    constexpr int IDX_STATIC = 0;
    constexpr int IDX_DYNAMIC = 1;

    static decal::DecalSystem* s_engine{ nullptr };
}

namespace decal
{
    void DecalSystem::init() noexcept
    {
        s_engine = new DecalSystem();
    }

    void DecalSystem::release() noexcept
    {
        auto* s = s_engine;
        s_engine = nullptr;
        delete s;
    }

    DecalSystem& DecalSystem::get() noexcept
    {
        assert(s_engine);
        return *s_engine;
    }
}

namespace decal {
    DecalSystem::DecalSystem()
    {
        m_collections.reserve(2);
        m_collections.emplace_back();
        m_collections.emplace_back();

        m_collections[IDX_DYNAMIC].setStatic(false);
    }

    DecalSystem::~DecalSystem() = default;

    void DecalSystem::clearWT()
    {
        ASSERT_WT();

        for (auto& coll : m_collections) {
            coll.clearWT();
        }
    }

    void DecalSystem::shutdownWT()
    {
        ASSERT_WT();

        clearWT();
    }

    void DecalSystem::prepareWT() {
        ASSERT_WT();

        const auto& assets = Assets::get();

        m_enabled = assets.decalEnabled;

        for (auto& coll : m_collections) {
            coll.prepareWT();
        }

        if (!isEnabled()) return;

        clearWT();
    }

    void DecalSystem::clearRT()
    {
        ASSERT_RT();

        for (auto& coll : m_collections) {
            coll.clearRT();
        }
    }

    void DecalSystem::shutdownRT()
    {
        ASSERT_RT();

        for (auto& coll : m_collections) {
            coll.shutdownRT();
        }
    }

    void DecalSystem::prepareRT()
    {
        ASSERT_RT();

        for (auto& coll : m_collections) {
            coll.prepareRT();
        }
    }

    void DecalSystem::addDecal(const Decal& decal)
    {
        if (!decal.m_parent) return;

        DecalCollection* coll = decal.m_static ?
            &m_collections[IDX_STATIC] :
            &m_collections[IDX_DYNAMIC];

        coll->addDecal(decal);
    }

    void DecalSystem::updateWT(const UpdateContext& ctx)
    {
        if (!isEnabled()) return;

        for (auto& coll : m_collections) {
            coll.updateWT(ctx);
        }
    }

    void DecalSystem::updateRT(const UpdateContext& ctx)
    {
        for (auto& coll : m_collections) {
            coll.updateRT(ctx);
        }
    }

    int DecalSystem::getActiveDecalCount() const noexcept
    {
        uint32_t decalCount = 0;
        for (const auto& coll : m_collections) {
            decalCount += coll.getActiveDecalCount();
        }
        return decalCount;
    }
}
