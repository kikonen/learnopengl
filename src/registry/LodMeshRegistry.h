#pragma once

#include <memory>
#include <vector>
#include <span>

#include "kigl/kigl.h"
#include "kigl/GLSyncQueue.h"

namespace mesh
{
    struct LodMesh;
    struct Transform;
    struct InstanceSSBO;
}

class LodMeshRegistry
{
public:
    LodMeshRegistry();
    ~LodMeshRegistry();

    uint32_t addTransforms(
        const std::span<mesh::LodMesh> lodMesh,
        const std::span<mesh::Transform> transforms);

    void removeLodMeshes(
        uint32_t index,
        size_t count);

    void prepareRT();
    void update();

private:
    void updateBuffer();
    void createInstanceBuffers(size_t totalCount);

private:
    std::vector<mesh::InstanceSSBO> m_instances;

    // { size: [index, ...] }
    std::unordered_map<size_t, std::vector<uint32_t>> m_freeSlots;

    std::unique_ptr<kigl::GLSyncQueue<mesh::InstanceSSBO>> m_instanceBuffers{ nullptr };

    bool m_useFenceDebug{ false };
    bool m_debug{ false };
};
