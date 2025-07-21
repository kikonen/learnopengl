#include "RenderData.h"

#include "kigl/GLSyncQueue_impl.h"

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

namespace {
    constexpr int PER_FRAME_COUNT = 3;
    constexpr int PER_CONTEXT_COUNT = 6;
}

namespace render {
    RenderData::RenderData()
        : m_lightsUBO{}
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

        m_camera = std::make_unique<kigl::GLSyncQueue<CameraUBO>>(
            "camera_ubo",
            1,
            PER_CONTEXT_COUNT,
            useMapped,
            useInvalidate,
            useFence,
            useFenceDebug);

        m_data = std::make_unique<kigl::GLSyncQueue<DataUBO>>(
            "data_ubo",
            1,
            PER_FRAME_COUNT,
            useMapped,
            useInvalidate,
            useFence,
            useFenceDebug);

        m_shadow = std::make_unique<kigl::GLSyncQueue<ShadowUBO>>(
            "shadow_ubo",
            1,
            PER_FRAME_COUNT,
            useMapped,
            useInvalidate,
            useFence,
            useFenceDebug);

        m_debug = std::make_unique<kigl::GLSyncQueue<DebugUBO>>(
            "debug_ubo",
            1,
            PER_FRAME_COUNT,
            useMapped,
            useInvalidate,
            useFence,
            useFenceDebug);

        m_bufferInfo = std::make_unique<kigl::GLSyncQueue<BufferInfoUBO>>(
            "buffer_info_ubo",
            1,
            PER_CONTEXT_COUNT,
            useMapped,
            useInvalidate,
            useFence,
            useFenceDebug);

        m_clipPlanes = std::make_unique<kigl::GLSyncQueue<ClipPlanesUBO>>(
            "cliplanes_ubo",
            1,
            PER_CONTEXT_COUNT,
            useMapped,
            useInvalidate,
            useFence,
            useFenceDebug);

        m_lights = std::make_unique<kigl::GLSyncQueue<LightsUBO>>(
            "lights_ubo",
            1,
            PER_FRAME_COUNT,
            useMapped,
            useInvalidate,
            useFence,
            useFenceDebug);

        m_camera->prepare(bufferAlignment, debug);
        m_data->prepare(bufferAlignment, debug);
        m_shadow->prepare(bufferAlignment, debug);
        m_debug->prepare(bufferAlignment, debug);
        m_bufferInfo->prepare(bufferAlignment, debug);
        m_clipPlanes->prepare(bufferAlignment, debug);
        m_lights->prepare(bufferAlignment, debug);
    }

    void RenderData::updateCamera(const CameraUBO& data)
    {
        auto& buffer = *m_camera;
        buffer.set(0, data);
        buffer.flush();
        buffer.bindCurrentUBO(UBO_CAMERA, false, 1);
        buffer.next();
    }

    void RenderData::updateData(const DataUBO& data)
    {
        auto& buffer = *m_data;
        buffer.set(0, data);
        buffer.flush();
        buffer.bindCurrentUBO(UBO_DATA, false, 1);
        buffer.next();
    }

    void RenderData::updateShadow(const ShadowUBO& data)
    {
        auto& buffer = *m_shadow;
        buffer.set(0, data);
        buffer.flush();
        buffer.bindCurrentUBO(UBO_SHADOW, false, 1);
        buffer.next();
    }

    void RenderData::updateDebug(const DebugUBO& data)
    {
        auto& buffer = *m_debug;
        buffer.set(0, data);
        buffer.flush();
        buffer.bindCurrentUBO(UBO_DEBUG, false, 1);
        buffer.next();
    }

    void RenderData::updateBufferInfo(const BufferInfoUBO& data)
    {
        auto& buffer = *m_bufferInfo;
        buffer.set(0, data);
        buffer.flush();
        buffer.bindCurrentUBO(UBO_BUFFER_INFO, false, 1);
        buffer.next();
    }

    void RenderData::updateClipPlanes(const ClipPlanesUBO& data)
    {
        auto& buffer = *m_clipPlanes;
        buffer.set(0, data);
        buffer.flush();
        buffer.bindCurrentUBO(UBO_CLIP_PLANES, false, 1);
        buffer.next();
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

        auto& data = m_lightsUBO;
        auto& buffer = *m_lights;
        buffer.set(0, data);
        buffer.flush();
        buffer.bindCurrentUBO(UBO_LIGHTS, false, 1);
        buffer.next();
    }

    void RenderData::invalidateAll()
    {
    }
}
