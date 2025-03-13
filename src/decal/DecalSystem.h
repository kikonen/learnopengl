#pragma once

#include <vector>

struct PrepareContext;
struct UpdateContext;

namespace decal {
    struct Decal;
    class DecalCollection;

    class DecalSystem final
    {
    public:
        static void init() noexcept;
        static void release() noexcept;
        static DecalSystem& get() noexcept;

        DecalSystem();
        ~DecalSystem();

        void clearWT();
        void shutdownWT();
        void prepareWT();

        void updateWT(const UpdateContext& ctx);

        void clearRT();
        void shutdownRT();
        void prepareRT();

        void updateRT(const UpdateContext& ctx);

        void setEnabled(bool enabled) {
            m_enabled = enabled;
        }

        bool isEnabled() const noexcept { return m_enabled; }

        void addDecal(const Decal& decal);

        const std::vector<DecalCollection>& getCollections() const
        {
            return m_collections;
        }

        int getActiveDecalCount() const noexcept;

    private:
        bool m_enabled{ false };

        std::vector<DecalCollection> m_collections;
    };
}
