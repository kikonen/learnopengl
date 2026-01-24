#include "RenderData.h"

#include "kigl/RingAllocator.h"

#include "shader/UBO.h"
#include "shader/CameraUBO.h"
#include "shader/DataUBO.h"
#include "shader/ShadowUBO.h"
#include "shader/DebugUBO.h"
#include "shader/BufferInfoUBO.h"
#include "shader/ClipPlaneUBO.h"
#include "shader/LightsUBO.h"

#include "material/ImageTexture.h"

#include "model/Node.h"
#include "component/Light.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"

#include "NodeCollection.h"

namespace
{
    // Estimate max UBO usage per frame:
    // - Camera: updated per render pass (cubemap, water, mirror, main, shadows, etc.)
    //   Estimate ~10 updates per frame
    // - Data, Shadow, Debug, Lights: typically 1 per frame
    // - BufferInfo, ClipPlanes: per context, ~6 per frame
    //
    // Conservative estimate per frame:
    constexpr size_t MAX_CAMERA_PER_FRAME = 16;
    constexpr size_t MAX_DATA_PER_FRAME = 2;
    constexpr size_t MAX_SHADOW_PER_FRAME = 2;
    constexpr size_t MAX_DEBUG_PER_FRAME = 2;
    constexpr size_t MAX_BUFFER_INFO_PER_FRAME = 8;
    constexpr size_t MAX_CLIP_PLANES_PER_FRAME = 8;
    constexpr size_t MAX_LIGHTS_PER_FRAME = 2;

    constexpr size_t estimateUboSizePerFrame()
    {
        return MAX_CAMERA_PER_FRAME * sizeof(CameraUBO)
            + MAX_DATA_PER_FRAME * sizeof(DataUBO)
            + MAX_SHADOW_PER_FRAME * sizeof(ShadowUBO)
            + MAX_DEBUG_PER_FRAME * sizeof(DebugUBO)
            + MAX_BUFFER_INFO_PER_FRAME * sizeof(BufferInfoUBO)
            + MAX_CLIP_PLANES_PER_FRAME * sizeof(ClipPlanesUBO)
            + MAX_LIGHTS_PER_FRAME * sizeof(LightsUBO)
            // Add 50% headroom for alignment padding
            + 16 * 256;  // ~16 allocations * 256 byte alignment worst case
    }
}

namespace render
{
    RenderData::RenderData()
        : m_lightsUBO{}
    {}

    RenderData::~RenderData()
    {
    }

    void RenderData::prepare(bool debug)
    {
        GLint bufferAlignment;
        glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &bufferAlignment);

        // Create ring allocator with UBO alignment
        m_ring = std::make_unique<kigl::RingAllocator>(
            "render_data_ubo",
            static_cast<size_t>(bufferAlignment),
            3,
            1.5f);

        m_ring->create(estimateUboSizePerFrame());
    }

    void RenderData::beginFrame()
    {
        m_ring->beginFrame();
    }

    void RenderData::endFrame()
    {
        m_ring->endFrame();
    }

    void RenderData::updateCamera(const CameraUBO& data)
    {
        auto alloc = m_ring->allocate<CameraUBO>();
        if (alloc) {
            *alloc = data;
            m_ring->flushRange(alloc.ref);
            m_ring->bindUBO(UBO_CAMERA, alloc.ref);
        }
    }

    void RenderData::updateData(const DataUBO& data)
    {
        m_lastDataUBO = data;
        auto alloc = m_ring->allocate<DataUBO>();
        if (alloc) {
            *alloc = data;
            m_ring->flushRange(alloc.ref);
            m_ring->bindUBO(UBO_DATA, alloc.ref);
        }
    }

    void RenderData::updateShadow(const ShadowUBO& data)
    {
        auto alloc = m_ring->allocate<ShadowUBO>();
        if (alloc) {
            *alloc = data;
            m_ring->flushRange(alloc.ref);
            m_ring->bindUBO(UBO_SHADOW, alloc.ref);
        }
    }

    void RenderData::updateDebug(const DebugUBO& data)
    {
        auto alloc = m_ring->allocate<DebugUBO>();
        if (alloc) {
            *alloc = data;
            m_ring->flushRange(alloc.ref);
            m_ring->bindUBO(UBO_DEBUG, alloc.ref);
        }
    }

    void RenderData::updateBufferInfo(const BufferInfoUBO& data)
    {
        auto alloc = m_ring->allocate<BufferInfoUBO>();
        if (alloc) {
            *alloc = data;
            m_ring->flushRange(alloc.ref);
            m_ring->bindUBO(UBO_BUFFER_INFO, alloc.ref);
        }
    }

    void RenderData::updateClipPlanes(const ClipPlanesUBO& data)
    {
        auto alloc = m_ring->allocate<ClipPlanesUBO>();
        if (alloc) {
            *alloc = data;
            m_ring->flushRange(alloc.ref);
            m_ring->bindUBO(UBO_CLIP_PLANES, alloc.ref);
        }
    }

    void RenderData::updateLights(NodeCollection* collection)
    {
        auto& lightsUbo = m_lightsUBO;

        {
            auto& handle = collection->getDirLightNode();
            auto* node = handle.toNode();

            if (node && node->m_light->m_enabled) {
                lightsUbo.u_dir[0] = node->m_light->toDirLightUBO();
                lightsUbo.u_dirCount = 1;
            }
            else {
                lightsUbo.u_dirCount = 0;
            }
        }

        {
            int count = 0;
            auto& handles = collection->getPointLightNodes();
            for (auto& handle : handles) {
                auto* node = handle.toNode();
                if (!node) continue;

                if (count >= MAX_LIGHT_COUNT) break;
                if (!node->m_light->m_enabled) continue;

                lightsUbo.u_pointLights[count] = node->m_light->toPointightUBO();
                count++;
            }
            lightsUbo.u_pointCount = count;

            const int diff = static_cast<int>(handles.size()) - MAX_LIGHT_COUNT;
            if (diff > 0) {
                KI_INFO_OUT(fmt::format("SKIPPED POINT_LIGHTS: {}", diff));
            }
        }

        {
            int count = 0;
            const auto& handles = collection->getSpotLightNodes();
            for (auto& handle : handles) {
                auto* node = handle.toNode();
                if (!node) continue;

                if (count >= MAX_LIGHT_COUNT) break;
                if (!node->m_light->m_enabled) continue;

                lightsUbo.u_spotLights[count] = node->m_light->toSpotLightUBO();
                count++;
            }
            lightsUbo.u_spotCount = count;

            const int diff = static_cast<int>(handles.size()) - MAX_LIGHT_COUNT;
            if (diff > 0) {
                KI_INFO_OUT(fmt::format("SKIPPED SPOT_LIGHTS: {}", diff));
            }
        }

        auto alloc = m_ring->allocate<LightsUBO>();
        if (alloc) {
            *alloc = lightsUbo;
            m_ring->flushRange(alloc.ref);
            m_ring->bindUBO(UBO_LIGHTS, alloc.ref);
        }
    }
}
