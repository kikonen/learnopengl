#include "RenderData.h"

#include "shader/UBO.h"
#include "shader/MatricesUBO.h"
#include "shader/CameraUBO.h"
#include "shader/DataUBO.h"
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

namespace {
    constexpr int RENDER_DATA_BUFFER_COUNT = 32;
}

namespace render {
    RenderData::RenderData()
        : m_matricesBuffer{ "matrices_ubo" },
        m_cameraBuffer{ "camera_ubo" },
        m_dataBuffer{ "data_ubo" },
        m_debugBuffer{ "debug_ubo" },
        m_bufferInfoBuffer{ "buffer_info_ubo" },
        m_clipPlanesBuffer{ "clip_planes_ubo" },
        m_lightsBuffer{ "lights_ubo" }
    {
    }

    void RenderData::prepare(
        bool useMapped,
        bool useInvalidate,
        bool useFence,
        bool useFenceDebug,
        bool debug)
    {
        int bufferAlignment;
        glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &bufferAlignment);

        m_matricesBuffer.create();
        m_matricesBuffer.initEmpty(sizeof(MatricesUBO), GL_DYNAMIC_STORAGE_BIT);

        m_cameraBuffer.create();
        m_cameraBuffer.initEmpty(sizeof(CameraUBO), GL_DYNAMIC_STORAGE_BIT);

        m_dataBuffer.create();
        m_dataBuffer.initEmpty(sizeof(DataUBO), GL_DYNAMIC_STORAGE_BIT);

        m_debugBuffer.create();
        m_debugBuffer.initEmpty(sizeof(DebugUBO), GL_DYNAMIC_STORAGE_BIT);

        m_bufferInfoBuffer.create();
        m_bufferInfoBuffer.initEmpty(sizeof(BufferInfoUBO), GL_DYNAMIC_STORAGE_BIT);

        m_clipPlanesBuffer.create();
        m_clipPlanesBuffer.initEmpty(sizeof(ClipPlanesUBO), GL_DYNAMIC_STORAGE_BIT);

        m_lightsBuffer.create();
        m_lightsBuffer.initEmpty(sizeof(LightsUBO), GL_DYNAMIC_STORAGE_BIT);
    }

    void RenderData::updateMatrices(const MatricesUBO& data)
    {
        constexpr auto sz = sizeof(MatricesUBO);
        m_matricesBuffer.update(0, sz, &data);
        m_matricesBuffer.bindUBORange(UBO_MATRICES, 0, sz);
    }

    void RenderData::updateCamera(const CameraUBO& data)
    {
        constexpr auto sz = sizeof(CameraUBO);
        m_cameraBuffer.update(0, sz, &data);
        m_cameraBuffer.bindUBORange(UBO_CAMERA, 0, sz);
    }

    void RenderData::updateData(const DataUBO& data)
    {
        constexpr auto sz = sizeof(DataUBO);
        m_dataBuffer.update(0, sz, &data);
        m_dataBuffer.bindUBORange(UBO_DATA, 0, sz);
    }

    void RenderData::updateDebug(const DebugUBO& data)
    {
        constexpr auto sz = sizeof(DebugUBO);
        m_debugBuffer.update(0, sz, &data);
        m_debugBuffer.bindUBORange(UBO_DEBUG, 0, sz);
    }

    void RenderData::updateBufferInfo(const BufferInfoUBO& data)
    {
        constexpr auto sz = sizeof(BufferInfoUBO);
        m_bufferInfoBuffer.update(0, sz, &data);
        m_bufferInfoBuffer.bindUBORange(UBO_BUFFER_INFO, 0, sz);
    }

    void RenderData::updateClipPlanes(const ClipPlanesUBO& data)
    {
        constexpr auto sz = sizeof(ClipPlanesUBO);
        m_clipPlanesBuffer.update(0, sz, &data);
        m_clipPlanesBuffer.bindUBORange(UBO_CLIP_PLANES, 0, sz);
    }

    void RenderData::updateLights(NodeCollection* collection)
    {
        auto & lightsUbo = m_lightsUBO;

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

        constexpr auto sz = sizeof(LightsUBO);
        m_lightsBuffer.update(0, sz, &m_lightsUBO);
        m_lightsBuffer.bindUBORange(UBO_LIGHTS, 0, sz);
    }
}
