#pragma once

#include <vector>
#include <mutex>
#include <atomic>
#include <memory>

#include "Decal.h"
#include "DecalSSBO.h"

#include "kigl/GLBuffer.h"

struct PrepareContext;
struct UpdateContext;
class RenderContext;

class Registry;

namespace decal {
    class DecalSystem;

    class DecalBuffer final
    {
    public:
        DecalBuffer(DecalSystem* decalSystem);
        ~DecalBuffer();

        void clear();
        void shutdown();
        void prepare();

        void update(const UpdateContext& ctx);

    private:
        void updateBuffer(
            const std::vector<DecalSSBO>& snapshot);

    private:
        DecalSystem* const m_decalSystem;

        kigl::GLBuffer m_ssbo{ "decal_ssbo" };
        size_t m_lastDecalSize{ 0 };

        bool m_useMapped{ false };
        bool m_useInvalidate{ false };
        bool m_useFence{ false };
        bool m_useDebugFence{ false };
    };
}
