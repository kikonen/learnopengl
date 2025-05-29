#pragma once

#include "kigl/GLSyncQueue.h"

class Registry;

struct MatricesUBO;
struct DataUBO;
struct DebugUBO;
struct BufferInfoUBO;
struct ClipPlanesUBO;
struct LightsUBO;

namespace render {
    class RenderData {
    public:
        RenderData();

        void prepare(
            bool useMapped,
            bool useInvalidate,
            bool useFence,
            bool useFenceDebug,
            bool debug);

        void bind();
        void update();

        void updateMatrices(const MatricesUBO& data);
        void updateData(const DataUBO& data);
        void updateDebug(const DebugUBO& data);
        void updateBufferInfo(const BufferInfoUBO& data);
        void updateClipPlanes(const ClipPlanesUBO& data);
        void updateLights(Registry* registry);

    public:

    private:
        std::unique_ptr<LightsUBO> m_lightsUbo;

        std::unique_ptr<kigl::GLSyncQueue<MatricesUBO>> m_matrices;
        std::unique_ptr<kigl::GLSyncQueue<DataUBO>> m_data;
        std::unique_ptr<kigl::GLSyncQueue<DebugUBO>> m_debug;
        std::unique_ptr<kigl::GLSyncQueue<BufferInfoUBO>> m_bufferInfo;
        std::unique_ptr<kigl::GLSyncQueue<ClipPlanesUBO>> m_clipPlanes;
        std::unique_ptr<kigl::GLSyncQueue<LightsUBO>> m_lights;
    };
}
