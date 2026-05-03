#include "DecalRegistry.h"

namespace {
    static decal::DecalRegistry* s_registry{ nullptr };
}

namespace decal {
    void DecalRegistry::init() noexcept
    {
        assert(!s_registry);
        s_registry = new DecalRegistry();
    }

    void DecalRegistry::release() noexcept
    {
        auto* s = s_registry;
        s_registry = nullptr;
        delete s;
    }

    DecalRegistry& DecalRegistry::get() noexcept
    {
        assert(s_registry);
        return *s_registry;
    }
}

namespace decal {
    DecalRegistry::DecalRegistry()
    {
        clear();
    }

    DecalRegistry::~DecalRegistry() = default;

    void DecalRegistry::clear()
    {
        m_definitions.clear();
        m_nameToIndex.clear();

        m_definitions.emplace_back();
        m_nameToIndex.insert({ 0, 0 });
    }

    void DecalRegistry::addDecal(const decal::DecalDefinition& df)
    {
        std::lock_guard lock{ m_lock };

        if (df.m_sid == 0) return;

        size_t index = m_definitions.size();
        m_definitions.push_back(df);
        m_nameToIndex.insert({ df.m_sid, index });
    }

    decal::DecalDefinition DecalRegistry::getDecal(ki::decal_id id)
    {
        std::lock_guard lock{ m_lock };

        const auto& it = m_nameToIndex.find(id);
        if (it == m_nameToIndex.end()) return {};
        return m_definitions[it->second];
    }

    std::vector<ki::decal_id> DecalRegistry::getDecalIds() const
    {
        std::vector<ki::decal_id> ids;

        {
            std::lock_guard lock{ m_lock };
            for (auto [k, v] : m_nameToIndex) {
                ids.push_back(k);
            }
        }

        return ids;
    }
}
