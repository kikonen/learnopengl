#pragma once

#include <string>
#include <vector>
#include <mutex>

#include "DecalDefinition.h"

namespace decal
{
    class DecalRegistry {
    public:
        static DecalRegistry& get();

        DecalRegistry();
        ~DecalRegistry();

        void clear();

        void addDecal(decal::DecalDefinition& df);

        // @return decal with null id if not valid
        decal::DecalDefinition getDecal(const ki::StringID& name);

        std::vector<ki::StringID> getDecalIds() const;

    private:
        mutable std::mutex m_lock{};

        std::vector<decal::DecalDefinition> m_definitions;
        std::unordered_map<ki::StringID, size_t> m_nameToIndex;
    };
}
