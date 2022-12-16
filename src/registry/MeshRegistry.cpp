#include "MeshRegistry.h"

#include "scene/RenderContext.h"
#include "asset/ModelMesh.h"
#include "asset/ModelMeshVBO.h"
#include "asset/MeshLoader.h"

namespace {
    constexpr int MAX_VERTEX_ENTRIES = 100000;

    constexpr size_t BUFFER_ENTRY_SIZE = sizeof(VertexEntry) + sizeof(IndexEntry);
    constexpr size_t BUFFER_SIZE = BUFFER_ENTRY_SIZE * MAX_VERTEX_ENTRIES;
}


MeshRegistry::MeshRegistry(const Assets& assets)
    : assets(assets)
{
}

MeshRegistry::~MeshRegistry() {
    delete[] m_buffer;
}

void MeshRegistry::registerMeshVBO(ModelMeshVBO& meshVBO)
{
    if (meshVBO.m_vertexEntries.empty()) return;

    {
        const int vertexSize = sizeof(VertexEntry) * meshVBO.m_vertexEntries.size();
        const int indexSize = sizeof(IndexEntry) * meshVBO.m_indexEntries.size();
        const int sz = vertexSize + indexSize;

        assert(m_bufferOffset + sz <= BUFFER_SIZE);

        meshVBO.m_vertexOffset = m_bufferOffset;
        meshVBO.m_indexOffset = m_bufferOffset + vertexSize;
        meshVBO.m_vbo = &m_vbo;

        {
            VertexEntry* vbo = (VertexEntry*)(m_buffer + meshVBO.m_vertexOffset);

            for (const auto& entry : meshVBO.m_vertexEntries) {
                *vbo = entry;
                vbo++;
            }
        }
        {
            IndexEntry* vbo = (IndexEntry*)(m_buffer + meshVBO.m_indexOffset);

            for (const auto& entry : meshVBO.m_indexEntries) {
                *vbo = entry;
                vbo++;
            }
        }

        m_vbo.update(
            m_bufferOffset,
            sz,
            m_buffer + m_bufferOffset);

        m_bufferOffset += sz;
    }
}

void MeshRegistry::prepare()
{
    {
        m_vbo.create();

        m_buffer = new unsigned char[BUFFER_SIZE];
        memset(m_buffer, 0, BUFFER_SIZE);

        m_vbo.initEmpty(BUFFER_SIZE, GL_DYNAMIC_STORAGE_BIT);
    }
}

ModelMesh* MeshRegistry::getMesh(
    const std::string& meshName)
{
    return getMesh(meshName, "");
}

ModelMesh* MeshRegistry::getMesh(
    const std::string& meshName,
    const std::string& meshPath)
{
    std::lock_guard<std::mutex> lock(m_meshes_lock);

    std::string key = meshPath + "/" + meshName;

    {
        const auto& e = m_meshes.find(key);
        if (e != m_meshes.end()) {
            return e->second.get();
        }
    }

    m_meshes[key] = std::make_unique<ModelMesh>(
        meshName,
        meshPath);
    const auto& e = m_meshes.find(key);
    auto* mesh = e->second.get();

    MeshLoader loader(assets);
    auto loaded = loader.load(*mesh, m_defaultMaterial.get(), m_forceDefaultMaterial);

    if (loaded) {
        loaded->prepareVolume();
    }

    return loaded;
}
