#pragma once

#include <vector>
#include <map>
#include <mutex>

#include "glm/glm.hpp"

#include "kigl/GLVertexArray.h"

#include "asset/Assets.h"
#include "asset/ModelVAO.h"

class Batch;
class Material;
class ModelMesh;
class ModelMeshVBO;

class ModelRegistry {
public:
    ModelRegistry(const Assets& assets);
    ~ModelRegistry();

    void prepare(Batch& batch);

    // @return VAO for mesh
    GLVertexArray* registerMeshVBO(ModelMeshVBO& meshVBO);

    ModelMesh* getMesh(
        const std::string& meshName);

    ModelMesh* getMesh(
        const std::string& meshName,
        const std::string& meshPath);

private:
    const Assets& assets;

    std::mutex m_meshes_lock;
    std::map<std::string, std::unique_ptr<ModelMesh>> m_meshes;

    std::unique_ptr<Material> m_defaultMaterial{ nullptr };
    bool m_forceDefaultMaterial = false;

    ModelVAO m_singleVAO{ true };
    ModelVAO m_multiVAO{ false };
};
