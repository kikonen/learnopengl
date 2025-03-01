#include "DecalRegistry.h"

namespace {
    decal::DecalRegistry g_instance;
}

namespace decal {
    DecalRegistry& DecalRegistry::get()
    {
        return g_instance;
    }

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
