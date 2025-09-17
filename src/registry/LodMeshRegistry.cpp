#include "LodMeshRegistry.h"

#include "asset/Assets.h"

#include "kigl/GLSyncQueue_impl.h"

#include "shader/SSBO.h"

#include "mesh/InstanceSSBO.h"

namespace {
    constexpr size_t INDEX_BLOCK_SIZE = 1000;
    constexpr size_t INDEX_BLOCK_COUNT = 500;

    constexpr size_t MAX_INDEX_COUNT = INDEX_BLOCK_SIZE * INDEX_BLOCK_COUNT;

    constexpr size_t MAX_INSTANCE_BUFFERS = 20;

    constexpr int BUFFER_ALIGNMENT = 1;
}

LodMeshRegistry::LodMeshRegistry()
{ }

LodMeshRegistry::~LodMeshRegistry() = default;

//uint32_t LodMeshRegistry::addLodMeshes(
//    std::span<mesh::LodMesh> lodMeshes)
//{
//    return 0;
//}

void LodMeshRegistry::removeLodMeshes(
    uint32_t index,
    size_t count)
{ }

void LodMeshRegistry::prepareRT()
{
    const auto& assets = Assets::get();

    m_debug = assets.batchBuffers;
    m_useFenceDebug = assets.glUseFenceDebug;
}

void LodMeshRegistry::updateBuffer()
{
    const size_t totalCount = m_instances.size();
    {
        createInstanceBuffers(totalCount);

        auto& current = m_instanceBuffers->current();
        auto* __restrict mappedData = m_instanceBuffers->currentMapped();

        m_instanceBuffers->waitFence();
        std::copy(
            std::begin(m_instances),
            std::end(m_instances),
            mappedData);
        m_instanceBuffers->bindCurrentSSBO(SSBO_INSTANCE_INDECES, false, totalCount);
    }
}

void LodMeshRegistry::createInstanceBuffers(size_t totalCount)
{
    if (!m_instanceBuffers || m_instanceBuffers->getEntryCount() < totalCount) {
        size_t blocks = (totalCount * 1.25f / INDEX_BLOCK_SIZE) + 2;
        size_t entryCount = blocks * INDEX_BLOCK_SIZE;

        m_instanceBuffers = std::make_unique<kigl::GLSyncQueue<mesh::InstanceSSBO>>(
            "mesh_instance",
            entryCount,
            MAX_INSTANCE_BUFFERS,
            true,
            false,
            true,
            m_useFenceDebug);
        m_instanceBuffers->prepare(BUFFER_ALIGNMENT, m_debug);
    }
}
