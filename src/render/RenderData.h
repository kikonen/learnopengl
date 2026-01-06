#pragma once

#include <memory>

#include "shader/LightsUBO.h"

struct CameraUBO;
struct DataUBO;
struct ShadowUBO;
struct DebugUBO;
struct BufferInfoUBO;
struct ClipPlanesUBO;
struct LightsUBO;

namespace kigl
{
    class RingAllocator;
}

namespace render
{
    class NodeCollection;

    class RenderData
    {
    public:
        RenderData();
        ~RenderData();

        void prepare(bool debug);

        void beginFrame();
        void endFrame();

        void updateCamera(const CameraUBO& data);
        void updateData(const DataUBO& data);
        void updateShadow(const ShadowUBO& data);
        void updateDebug(const DebugUBO& data);
        void updateBufferInfo(const BufferInfoUBO& data);
        void updateClipPlanes(const ClipPlanesUBO& data);
        void updateLights(NodeCollection* collection);

    private:
        std::unique_ptr<kigl::RingAllocator> m_ring;

        LightsUBO m_lightsUBO{};
    };
}
