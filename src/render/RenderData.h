#pragma once

#include <memory>

#include "shader/DataUBO.h"
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
        const DataUBO& getLastDataUBO() const { return m_lastDataUBO; }
        void updateShadow(const ShadowUBO& data);
        void updateDebug(const DebugUBO& data);
        void updateBufferInfo(const BufferInfoUBO& data);
        void updateClipPlanes(const ClipPlanesUBO& data);
        void updateLights(NodeCollection* collection);

    private:
        std::unique_ptr<kigl::RingAllocator> m_ring;

        DataUBO m_lastDataUBO{};
        LightsUBO m_lightsUBO{};
    };
}
