#pragma once

#include <vector>
#include <mutex>
#include <atomic>
#include <span>

#include "kigl/GLBuffer.h"


namespace animation {
    struct RigContainer;
    struct BoneTransform;
}

struct UpdateContext;
class RenderContext;
struct BoneTransformSSBO;

class BoneRegistry {
public:
    static BoneRegistry& get() noexcept;

    BoneRegistry();
    BoneRegistry& operator=(const BoneRegistry&) = delete;

    ~BoneRegistry();

    void prepare();

    // Register node instance specific rig
    // @return instance index into bone transform buffer
    uint32_t registerInstance(animation::RigContainer& rig);

    std::span<animation::BoneTransform> modifyRange(uint32_t start, uint32_t count) noexcept;

    uint32_t getActiveBoneCount() const noexcept;

    void updateWT(const UpdateContext& ctx);
    void updateRT(const UpdateContext& ctx);

private:
    void snapshotBones();
    void updateBuffer();

private:
    std::mutex m_lock{};
    std::mutex m_snapshotLock{};

    std::atomic_bool m_updateReady{ false };

    size_t m_lastSize{ 0 };

    std::vector<animation::BoneTransform> m_transforms;

    std::vector<BoneTransformSSBO> m_snapshot;

    kigl::GLBuffer m_ssbo{ "bone_transforms_ssbo" };
};
