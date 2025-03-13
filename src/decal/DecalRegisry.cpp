#include "DecalRegistry.h"

namespace {
    static decal::DecalRegistry* s_registry{ nullptr };
}

namespace decal {
    void DecalRegistry::init() noexcept
    {
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

    void DecalRegistry::addDecal(decal::DecalDefinition& df)
    {
        std::lock_guard lock{ m_lock };

        if (df.m_sid.empty()) return;

        size_t index = m_definitions.size();
        m_definitions.push_back(df);
        m_nameToIndex.insert({ df.m_sid, index });
    }

    decal::DecalDefinition DecalRegistry::getDecal(const ki::StringID& name)
    {
        std::lock_guard lock{ m_lock };

        const auto& it = m_nameToIndex.find(name);
        if (it == m_nameToIndex.end()) return {};
        return m_definitions[it->second];
    }

    std::vector<ki::StringID> DecalRegistry::getDecalIds() const
    {
        std::vector<ki::StringID> ids;

        {
            std::lock_guard lock{ m_lock };
            for (auto [k, v] : m_nameToIndex) {
                ids.push_back(k);
            }
        }

        return ids;
    }
}
