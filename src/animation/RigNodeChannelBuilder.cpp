#include "RigNodeChannelBuilder.h"

#include <set>

#include <assimp/scene.h>

#include "util/assimp_util.h"

#include "RigNodeChannel.h"

namespace animation
{
    RigNodeChannelBuilder::RigNodeChannelBuilder(RigNodeChannel& channel)
        : m_channel{ channel }
    {}

    void RigNodeChannelBuilder::reservePositionKeys(uint16_t size)
    {
        m_positionValues.reserve(size);
        m_positionKeyTimes.reserve(size);
    }

    void RigNodeChannelBuilder::reserveRotationKeys(uint16_t size)
    {
        m_rotationValues.reserve(size);
        m_rotationKeyTimes.reserve(size);
    }

    void RigNodeChannelBuilder::reserveScaleKeys(uint16_t size)
    {
        m_scaleValues.reserve(size);
        m_scaleKeyTimes.reserve(size);
    }

    void RigNodeChannelBuilder::addPositionKey(const aiVectorKey& key)
    {
        m_positionValues.push_back(assimp_util::toVec3(key.mValue));
        m_positionKeyTimes.push_back(static_cast<float>(key.mTime));
    }

    void RigNodeChannelBuilder::addRotationKey(const aiQuatKey& key)
    {
        m_rotationValues.push_back(assimp_util::toQuat(key.mValue));
        m_rotationKeyTimes.push_back(static_cast<float>(key.mTime));
    }

    void RigNodeChannelBuilder::addScaleKey(const aiVectorKey& key)
    {
        m_scaleValues.push_back(assimp_util::toVec3(key.mValue));
        m_scaleKeyTimes.push_back(static_cast<float>(key.mTime));
    }

    glm::vec3 RigNodeChannelBuilder::sampleVector(
        const std::vector<glm::vec3>& values,
        const std::vector<float>& times,
        float t) noexcept
    {
        if (values.size() == 1) {
            return values[0];
        }

        // Find surrounding keyframes
        size_t i = 0;
        while (i < times.size() - 1 && times[i + 1] <= t) {
            ++i;
        }

        if (i >= values.size() - 1) {
            return values.back();
        }

        const float t0 = times[i];
        const float t1 = times[i + 1];
        const float deltaTime = t1 - t0;

        if (deltaTime <= 0.0f) {
            return values[i];
        }

        const float factor = (t - t0) / deltaTime;
        return values[i] + factor * (values[i + 1] - values[i]);
    }

    glm::quat RigNodeChannelBuilder::sampleQuaternion(
        const std::vector<glm::quat>& values,
        const std::vector<float>& times,
        float t) noexcept
    {
        if (values.size() == 1) {
            return values[0];
        }

        // Find surrounding keyframes
        size_t i = 0;
        while (i < times.size() - 1 && times[i + 1] <= t) {
            ++i;
        }

        if (i >= values.size() - 1) {
            return values.back();
        }

        const float t0 = times[i];
        const float t1 = times[i + 1];
        const float deltaTime = t1 - t0;

        if (deltaTime <= 0.0f) {
            return values[i];
        }

        const float factor = (t - t0) / deltaTime;
        return glm::slerp(values[i], values[i + 1], factor);
    }

    void RigNodeChannelBuilder::unifyKeyTimes()
    {
        // Preserve original tracks for tick-based clip LUT generation
        // These are needed to sample without cross-clip interpolation artifacts
        m_channel.m_origPositionTimes = std::move(m_positionKeyTimes);
        m_channel.m_origRotationTimes = std::move(m_rotationKeyTimes);
        m_channel.m_origScaleTimes = std::move(m_scaleKeyTimes);
        m_channel.m_origPositionValues = std::move(m_positionValues);
        m_channel.m_origRotationValues = std::move(m_rotationValues);
        m_channel.m_origScaleValues = std::move(m_scaleValues);

        // Collect all unique time points from position, rotation, scale
        std::set<float> uniqueTimes;

        for (float t : m_channel.m_origPositionTimes) uniqueTimes.insert(t);
        for (float t : m_channel.m_origRotationTimes) uniqueTimes.insert(t);
        for (float t : m_channel.m_origScaleTimes) uniqueTimes.insert(t);

        // Build unified timeline directly into channel
        m_channel.m_keyTimes.assign(uniqueTimes.begin(), uniqueTimes.end());

        // Resample each track to unified timeline
        m_channel.m_positionKeyValues.clear();
        m_channel.m_rotationKeyValues.clear();
        m_channel.m_scaleKeyValues.clear();

        m_channel.m_positionKeyValues.reserve(m_channel.m_keyTimes.size());
        m_channel.m_rotationKeyValues.reserve(m_channel.m_keyTimes.size());
        m_channel.m_scaleKeyValues.reserve(m_channel.m_keyTimes.size());

        for (float t : m_channel.m_keyTimes) {
            m_channel.m_positionKeyValues.push_back(sampleVector(m_channel.m_origPositionValues, m_channel.m_origPositionTimes, t));
            m_channel.m_rotationKeyValues.push_back(sampleQuaternion(m_channel.m_origRotationValues, m_channel.m_origRotationTimes, t));
            m_channel.m_scaleKeyValues.push_back(sampleVector(m_channel.m_origScaleValues, m_channel.m_origScaleTimes, t));
        }
    }
}
