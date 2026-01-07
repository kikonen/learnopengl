#pragma once

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

struct aiNodeAnim;
struct aiVectorKey;
struct aiQuatKey;

namespace animation
{
    struct RigNodeChannel;

    // Builder for RigNodeChannel - handles loading and unifying key times
    // Separates import-time logic from runtime RigNodeChannel
    class RigNodeChannelBuilder {
    public:
        explicit RigNodeChannelBuilder(RigNodeChannel& channel);

        void reservePositionKeys(uint16_t size);
        void reserveRotationKeys(uint16_t size);
        void reserveScaleKeys(uint16_t size);

        void addPositionKey(const aiVectorKey& key);
        void addRotationKey(const aiQuatKey& key);
        void addScaleKey(const aiVectorKey& key);

        // Unify position/rotation/scale to common timeline by resampling
        // Finalizes the channel - builder should not be used after this
        void unifyKeyTimes();

    private:
        static glm::vec3 sampleVector(
            const std::vector<glm::vec3>& values,
            const std::vector<float>& times,
            float t) noexcept;

        static glm::quat sampleQuaternion(
            const std::vector<glm::quat>& values,
            const std::vector<float>& times,
            float t) noexcept;

        RigNodeChannel& m_channel;

        // Temporary storage during building
        std::vector<glm::vec3> m_positionValues;
        std::vector<glm::quat> m_rotationValues;
        std::vector<glm::vec3> m_scaleValues;

        std::vector<float> m_positionKeyTimes;
        std::vector<float> m_rotationKeyTimes;
        std::vector<float> m_scaleKeyTimes;
    };
}
