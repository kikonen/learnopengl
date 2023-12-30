#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <mutex>
#include <future>

#include "glm/glm.hpp"

#include "util/Util.h"
#include "kigl/GLVertexArray.h"

#include "asset/Assets.h"
#include "asset/ModelVAO.h"

struct Material;
class ModelMesh;
class ModelMeshVBO;
class UpdateContext;

class ModelRegistry {
public:
    ModelRegistry(
        const Assets& assets,
        std::shared_ptr<std::atomic<bool>> alive);

    ~ModelRegistry();

    void prepare();

    void updateRT(const UpdateContext& ctx);

    // @return VAO for mesh
    GLVertexArray* registerMeshVBO(ModelMeshVBO& meshVBO);

    std::shared_future<ModelMesh*> getMesh(
        std::string_view meshName,
        std::string_view rootDir);

    std::shared_future<ModelMesh*> getMesh(
        std::string_view meshName,
        std::string_view rootDir,
        std::string_view meshPath);

private:
    std::shared_future<ModelMesh*> startLoad(ModelMesh* mesh);

private:
    const Assets& m_assets;

    std::shared_ptr<std::atomic<bool>> m_alive;

    std::mutex m_meshes_lock{};
    std::unordered_map<const std::string, std::shared_future<ModelMesh*>, util::constant_string_hash> m_meshes;

    std::unique_ptr<Material> m_defaultMaterial{ nullptr };
    bool m_forceDefaultMaterial = false;

    ModelVAO m_vao{};
};
