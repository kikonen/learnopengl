#pragma once

#include <vector>
#include <map>
#include <mutex>

#include "glm/glm.hpp"

#include "kigl/GLBuffer.h"

#include "asset/Assets.h"

class Material;
class ModelMesh;
class ModelMeshVBO;

class MeshRegistry {
public:
    MeshRegistry(const Assets& assets);
    ~MeshRegistry();

    void prepare();
    void registerMeshVBO(ModelMeshVBO& meshVBO);

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

    unsigned char* m_buffer{ nullptr };
    int m_bufferOffset = 0;

    GLBuffer m_vbo;
};
