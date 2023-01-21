#pragma once

#include <vector>
#include <map>
#include <mutex>
#include <future>

#include "glm/glm.hpp"

#include "kigl/GLVertexArray.h"

#include "asset/Assets.h"
#include "asset/ModelVAO.h"

class Batch;
struct Material;
class ModelMesh;
class ModelMeshVBO;

class ModelRegistry {
public:
    ModelRegistry(
        const Assets& assets,
        std::shared_ptr<std::atomic<bool>> alive);

    ~ModelRegistry();

    void prepare(Batch& batch);

    // @return VAO for mesh
    GLVertexArray* registerMeshVBO(ModelMeshVBO& meshVBO);

    std::shared_future<ModelMesh*> getMesh(
        const std::string& meshName);

    std::shared_future<ModelMesh*> getMesh(
        const std::string& meshName,
        const std::string& meshPath);

private:
    std::shared_future<ModelMesh*> startLoad(ModelMesh* mesh);

private:
    const Assets& m_assets;

    std::shared_ptr<std::atomic<bool>> m_alive;

    std::mutex m_meshes_lock;
    std::map<const std::string, std::shared_future<ModelMesh*>> m_meshes;

    std::unique_ptr<Material> m_defaultMaterial{ nullptr };
    bool m_forceDefaultMaterial = false;

    ModelVAO m_singleVAO{ true };
    ModelVAO m_multiVAO{ false };
};
