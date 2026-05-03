#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>

#include "DecalDefinition.h"

namespace decal
{
    class DecalRegistry {
    public:
        static void init() noexcept;
        static void release() noexcept;
        static DecalRegistry& get() noexcept;

        DecalRegistry();
        ~DecalRegistry();

        void clear();

        void addDecal(const decal::DecalDefinition& df);

        // @return decal with null id if not valid
        decal::DecalDefinition getDecal(ki::decal_id sid);

        std::vector<ki::decal_id> getDecalIds() const;

    private:
        mutable std::mutex m_lock{};

        std::vector<decal::DecalDefinition> m_definitions;
        std::unordered_map<ki::decal_id, size_t> m_nameToIndex;
    };
}
