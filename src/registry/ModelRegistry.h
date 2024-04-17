#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <mutex>
#include <future>

#include "glm/glm.hpp"

#include "util/Util.h"
#include "kigl/GLVertexArray.h"

#include "mesh/ModelVAO.h"

struct Material;
struct UpdateContext;

namespace mesh {
    class ModelMesh;
}

class ModelRegistry {
public:
    static ModelRegistry& get() noexcept;

    ModelRegistry();
    ModelRegistry& operator=(const ModelRegistry&) = delete;

    ~ModelRegistry();

    void prepare(std::shared_ptr<std::atomic<bool>> alive);

    void updateRT(const UpdateContext& ctx);

    // @return VAO for mesh
    kigl::GLVertexArray* registerToVao(
        mesh::ModelMesh* mesh);

    std::shared_future<mesh::ModelMesh*> getMesh(
        std::string_view meshName,
        std::string_view rootDir);

    std::shared_future<mesh::ModelMesh*> getMesh(
        std::string_view meshName,
        std::string_view rootDir,
        std::string_view meshPath);

private:
    std::shared_future<mesh::ModelMesh*> startLoad(mesh::ModelMesh* mesh);

private:
    std::shared_ptr<std::atomic<bool>> m_alive;

    std::mutex m_meshes_lock{};
    std::unordered_map<const std::string, std::shared_future<mesh::ModelMesh*>, util::constant_string_hash> m_meshes;

    std::unique_ptr<Material> m_defaultMaterial{ nullptr };
    bool m_forceDefaultMaterial = false;

    mesh::ModelVAO m_vao{ "model" };
};
