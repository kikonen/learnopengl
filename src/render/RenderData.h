#pragma once

#include "kigl/GLSyncQueue.h"

#include "shader/LightsUBO.h"

class Registry;

struct CameraUBO;
struct DataUBO;
struct ShadowUBO;
struct DebugUBO;
struct BufferInfoUBO;
struct ClipPlanesUBO;
struct LightsUBO;

namespace render {
    class NodeCollection;

    class RenderData {
    public:
        RenderData();

        void prepare(
            bool useMapped,
            bool useInvalidate,
            bool useFence,
            bool useFenceDebug,
            bool debug);

        void updateCamera(const CameraUBO& data);
        void updateData(const DataUBO& data);
        void updateShadow(const ShadowUBO& data);
        void updateDebug(const DebugUBO& data);
        void updateBufferInfo(const BufferInfoUBO& data);
        void updateClipPlanes(const ClipPlanesUBO& data);
        void updateLights(NodeCollection* collection);

        void invalidateAll();

    private:
        LightsUBO m_lightsUBO;

        // NOTE KI OpenGL Insights - Chapter 28
        std::unique_ptr<kigl::GLSyncQueue<CameraUBO>> m_camera;
        std::unique_ptr<kigl::GLSyncQueue<DataUBO>> m_data;
        std::unique_ptr<kigl::GLSyncQueue<ShadowUBO>> m_shadow;
        std::unique_ptr<kigl::GLSyncQueue<DebugUBO>> m_debug;
        std::unique_ptr<kigl::GLSyncQueue<BufferInfoUBO>> m_bufferInfo;
        std::unique_ptr<kigl::GLSyncQueue<ClipPlanesUBO>> m_clipPlanes;
        std::unique_ptr<kigl::GLSyncQueue<LightsUBO>> m_lights;
    };
}
