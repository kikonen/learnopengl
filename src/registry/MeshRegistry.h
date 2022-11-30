#pragma once

#include <vector>
#include <map>
#include <mutex>

#include "asset/Assets.h"

class Material;
class ModelMesh;

class MeshRegistry {
public:
    MeshRegistry(const Assets& assets);
    ~MeshRegistry();

    ModelMesh* getMesh(
        const std::string& meshName);

    ModelMesh* getMesh(
        const std::string& meshName,
        const std::string& meshPath);

private:
    const Assets& assets;

    std::mutex m_meshes_lock;
    std::map<std::string, std::unique_ptr<ModelMesh>> m_meshes;

    int m_updatedSize = 0;

private:
    std::unique_ptr<Material> m_defaultMaterial{ nullptr };
    bool m_forceDefaultMaterial = false;
};
