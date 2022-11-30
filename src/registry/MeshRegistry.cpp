#include "MeshRegistry.h"

#include "scene/RenderContext.h"
#include "asset/ModelMesh.h"
#include "asset/MeshLoader.h"


MeshRegistry::MeshRegistry(const Assets& assets)
    : assets(assets)
{
}

MeshRegistry::~MeshRegistry() {
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
