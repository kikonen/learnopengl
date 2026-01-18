#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <Jolt/Jolt.h>
#include <Jolt/Math/Vec3.h>
#include <Jolt/Math/Quat.h>
#include <Jolt/Math/Real.h>

#include "JoltFoundation.h"
#include "Category.h"
#include "physics_util.h"
#include "size.h"

namespace physics {

// GLM to Jolt conversions
inline JPH::Vec3 toJolt(const glm::vec3& v) {
    return JPH::Vec3(v.x, v.y, v.z);
}

inline JPH::RVec3 toJoltR(const glm::vec3& v) {
    return JPH::RVec3(v.x, v.y, v.z);
}

// Note: GLM quaternion is (w, x, y, z), Jolt is (x, y, z, w) in constructor
// but stored as (x, y, z, w) internally
inline JPH::Quat toJolt(const glm::quat& q) {
    return JPH::Quat(q.x, q.y, q.z, q.w);
}

// Jolt to GLM conversions
inline glm::vec3 fromJolt(const JPH::Vec3& v) {
    return glm::vec3(v.GetX(), v.GetY(), v.GetZ());
}

// In single precision mode, RVec3 = Vec3, so only define this for double precision
#ifdef JPH_DOUBLE_PRECISION
inline glm::vec3 fromJoltR(const JPH::RVec3& v) {
    return glm::vec3(
        static_cast<float>(v.GetX()),
        static_cast<float>(v.GetY()),
        static_cast<float>(v.GetZ()));
}
#else
inline glm::vec3 fromJoltR(const JPH::RVec3& v) {
    return fromJolt(v);
}
#endif

inline glm::quat fromJolt(const JPH::Quat& q) {
    return glm::quat(q.GetW(), q.GetX(), q.GetY(), q.GetZ());
}

// Determine object layer based on motion type and category mask
inline JPH::ObjectLayer toObjectLayer(
    physics::Category category,
    bool isKinematic,
    bool isDynamic) {
    // Sensors (rays) go to sensor layer
    if (category == Category::ray) {
        return ObjectLayers::SENSOR;
    }

    // Dynamic bodies go to moving layer
    if (isDynamic && !isKinematic) {
        return ObjectLayers::MOVING;
    }

    // Static/kinematic terrain, scenery, ground -> non-moving
    if (category == Category::terrain ||
        category == Category::ground ||
        category == Category::scenery) {
        return ObjectLayers::NON_MOVING;
    }

    // Kinematic bodies (player, etc.) go to moving layer
    if (isKinematic) {
        return ObjectLayers::MOVING;
    }

    // Default to moving
    return ObjectLayers::MOVING;
}

// Determine motion type from body properties
inline JPH::EMotionType toMotionType(bool isKinematic, bool isDynamic) {
    if (!isDynamic) {
        return JPH::EMotionType::Static;
    }
    if (isKinematic) {
        return JPH::EMotionType::Kinematic;
    }
    return JPH::EMotionType::Dynamic;
}

// User data encoding/decoding
// We pack object_id and collision masks into the 64-bit user data field
// Layout: [32-bit object_id][16-bit category mask (low bits)][16-bit collision mask (low bits)]
// Note: Full 32-bit masks are not fully preserved, but top bits are rarely used

inline uint64_t packUserData(
    physics::object_id objectId,
    physics::Category category,
    uint32_t collisionMask)
{
    uint64_t data = static_cast<uint64_t>(objectId) << 32;
    data |= (static_cast<uint64_t>(mask(category) & 0xFFFF) << 16);
    data |= static_cast<uint64_t>(collisionMask & 0xFFFF);
    return data;
}

inline physics::object_id unpackObjectId(uint64_t userData) {
    return static_cast<physics::object_id>(userData >> 32);
}

inline physics::Category unpackCategory(uint64_t userData) {
    return static_cast<physics::Category>((userData >> 16) & 0xFFFF);
}

inline uint32_t unpackCollisionMask(uint64_t userData) {
    return static_cast<uint32_t>(userData & 0xFFFF);
}

} // namespace physics
