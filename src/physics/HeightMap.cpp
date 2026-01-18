#include "HeightMap.h"

#include <memory>
#include <algorithm>

#include "util/Log.h"
#include <fmt/format.h>

#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/Shape/HeightFieldShape.h>

#include "material/Image.h"

#include "model/Node.h"

#include "physics/Object.h"
#include "physics/jolt_util.h"
#include "physics/JoltFoundation.h"

namespace physics {
    HeightMap::HeightMap()
    {}

    HeightMap::HeightMap(HeightMap&& o) noexcept
        : m_id{ o.m_id },
        m_origin{ o.m_origin },
        m_worldTileSize{ o.m_worldTileSize },
        m_worldSizeU{ o.m_worldSizeU },
        m_worldSizeV{ o.m_worldSizeV },
        m_verticalRange{ o.m_verticalRange },
        m_horizontalScale{ o.m_horizontalScale },
        m_dataDepth{ o.m_dataDepth },
        m_dataWidth{ o.m_dataWidth },
        m_heightData{ o.m_heightData },
        m_bodyId{ o.m_bodyId },
        m_heightFieldShape{ std::move(o.m_heightFieldShape) }
    {
        // NOTE KI o is moved now
        o.m_heightData = nullptr;
        o.m_bodyId = JPH::BodyID();
    }

    HeightMap::~HeightMap()
    {
        if (m_heightData) {
            delete[] m_heightData;
        }

        // Body cleanup handled by PhysicsSystem
    }

    void HeightMap::prepare(
        const Image* _image,
        bool flipH)
    {
        m_prepared = true;
        m_flipH = flipH;

        const auto& image = *_image;

        const int imageH = image.m_height;
        const int imageW = image.m_width;
        const int channels = image.m_channels;

        const bool image16b = image.m_is16Bbit;
        const int entrySize = channels * (image16b ? 2 : 1);
        const float entryScale = image16b ? 65535.f : 255.f;

        const size_t size = imageH * imageW;

        const float rangeYmin = m_verticalRange[0];
        const float rangeYmax = m_verticalRange[1];
        const float rangeY = rangeYmax - rangeYmin;

        int minH = 9999999;
        int maxH = -1;
        float minY = 99999999.f;
        float maxY = -1.f;

        m_heightData = new float[size];

        const unsigned char* ptr = image.m_data;
        for (int vi = 0; vi < imageH; vi++) {
            const int v = m_flipH ? imageH - vi - 1 : vi;

            for (int u = 0; u < imageW; u++) {
                unsigned short heightValue = *((unsigned short*)ptr);
                float y = rangeYmin + ((float)heightValue / entryScale) * rangeY;

                if (heightValue < minH) minH = heightValue;
                if (heightValue > maxH) maxH = heightValue;
                if (y < minY) minY = y;
                if (y > maxY) maxY = y;

                m_heightData[u + v * imageW] = y;

                ptr += entrySize;
            }
        }

        KI_INFO_OUT(fmt::format(
            "HMAP: {} .. {} vs {} .. {}",
            minH, maxH, minY, maxY
        ));

        m_minH = minH;
        m_maxH = maxH;
        m_minY = minY;
        m_maxY = maxY;

        m_dataWidth = imageW;
        m_dataDepth = imageH;
    }

    void HeightMap::create(
        JPH::PhysicsSystem& physicsSystem,
        physics::Object& object)
    {
        if (!m_prepared || !m_heightData) return;

        auto& shape = object.m_shape;

        // Calculate scale factors
        // Jolt HeightFieldShape expects samples in a grid where spacing is determined by scale
        float scaleX = static_cast<float>(m_worldSizeU) / static_cast<float>(m_dataWidth - 1);
        float scaleZ = static_cast<float>(m_worldSizeV) / static_cast<float>(m_dataDepth - 1);

        // Create height field shape settings
        // Note: Jolt HeightFieldShape constructor wants:
        // - samples: height data (row-major order)
        // - offset: world space offset for the shape
        // - scale: scale in X, Y (height), Z directions
        // - sampleCount: must be power of 2 + 1 for optimal performance, but others work

        JPH::HeightFieldShapeSettings settings(
            m_heightData,
            JPH::Vec3::sZero(),  // Offset (position set via body)
            JPH::Vec3(scaleX, 1.0f, scaleZ),  // Scale - height is already in world units
            m_dataWidth);  // Sample count (assumes square for Jolt, uses width)

        // Set some additional settings
        settings.mBitsPerSample = 8;  // 8 bits per sample for reasonable precision
        settings.mBlockSize = 4;  // Block size for compression

        // Create the shape
        JPH::Shape::ShapeResult result = settings.Create();

        if (result.HasError()) {
            KI_CRITICAL(fmt::format("HMAP: Failed to create HeightFieldShape: {}", result.GetError().c_str()));
            return;
        }

        m_heightFieldShape = result.Get();

        // Create static body for the terrain
        JPH::BodyCreationSettings bodySettings(
            m_heightFieldShape,
            JPH::RVec3::sZero(),  // Position will be set below
            JPH::Quat::sIdentity(),
            JPH::EMotionType::Static,
            ObjectLayers::NON_MOVING);

        bodySettings.mFriction = 0.5f;
        bodySettings.mRestitution = 0.0f;

        // Pack user data
        bodySettings.mUserData = packUserData(0, shape.category, shape.collisionMask);

        JPH::BodyInterface& bodyInterface = physicsSystem.GetBodyInterface();
        m_bodyId = bodyInterface.CreateAndAddBody(bodySettings, JPH::EActivation::DontActivate);

        // Store the body ID in the shape for position updates
        shape.m_staticBodyId = m_bodyId;
        shape.m_heightFieldShape = m_heightFieldShape;

        KI_INFO_OUT(fmt::format(
            "HMAP: Created Jolt HeightFieldShape {}x{}, world size {}x{}, scale {:.2f}x{:.2f}",
            m_dataWidth, m_dataDepth, m_worldSizeU, m_worldSizeV, scaleX, scaleZ
        ));
    }

    float HeightMap::getTerrainHeight(float u, float v) const noexcept
    {
        if (m_dataDepth == 0 || m_dataWidth == 0) return 0;

        // NOTE KI use bilinear interpolation
        // use "clamp to edge"

        const float mapX = ((float)(m_dataWidth)) * u;
        const float mapY = ((float)(m_dataDepth)) * (1.f - v);

        // floor
        int x = static_cast<int>(mapX);
        int y = static_cast<int>(mapY);

        const float fractX = mapX - x;
        const float fractY = mapY - y;

        x = std::clamp(x, 0, m_dataWidth - 1);
        y = std::clamp(y, 0, m_dataDepth - 1);

        int nextX = std::clamp(x + 1, 0, m_dataWidth - 1);
        int nextY = std::clamp(y + 1, 0, m_dataDepth - 1);

        const float h00 = m_heightData[m_dataWidth * y       + x];
        const float h10 = m_heightData[m_dataWidth * nextY   + x];
        const float h01 = m_heightData[m_dataWidth * y       + nextX];
        const float h11 = m_heightData[m_dataWidth * nextY   + nextX];

        const float bottomH = (h01 - h00) * fractX + h00;
        const float topH    = (h11 - h10) * fractX + h10;

        const float finalH = (topH - bottomH) * fractY + bottomH;

        return finalH;
    }

    float HeightMap::getLevel(const glm::vec3& worldPos) const noexcept
    {
        const auto& state = m_origin->getState();
        const auto& originPos = state.getWorldPosition();
        const auto& modelMat = state.getModelMatrix();

        const auto diff = worldPos - originPos;

        const float u = diff.x / (float)m_worldSizeU;
        const float v = 1.f - diff.z / (float)m_worldSizeV;

        const float h = getTerrainHeight(u, v);

        const auto p = glm::vec4{ 0.f, h, 0.f, 1.f };

        const auto newWorldPos = modelMat * p;
        return newWorldPos.y;
    }
}
