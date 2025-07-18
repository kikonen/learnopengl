#pragma once

#include "kigl/GLBuffer.h"

#include "shader/LightsUBO.h"

class Registry;

struct MatricesUBO;
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

        void updateMatrices(const MatricesUBO& data);
        void updateCamera(const CameraUBO& data);
        void updateData(const DataUBO& data);
        void updateShadow(const ShadowUBO& data);
        void updateDebug(const DebugUBO& data);
        void updateBufferInfo(const BufferInfoUBO& data);
        void updateClipPlanes(const ClipPlanesUBO& data);
        void updateLights(NodeCollection* collection);

    private:
        LightsUBO m_lightsUBO;

        kigl::GLBuffer m_matricesBuffer;
        kigl::GLBuffer m_cameraBuffer;
        kigl::GLBuffer m_shadowBuffer;
        kigl::GLBuffer m_dataBuffer;
        kigl::GLBuffer m_debugBuffer;
        kigl::GLBuffer m_bufferInfoBuffer;
        kigl::GLBuffer m_clipPlanesBuffer;
        kigl::GLBuffer m_lightsBuffer;
    };
}
