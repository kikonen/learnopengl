#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <mutex>
#include <future>

#include "glm/glm.hpp"

#include "util/Util.h"
#include "kigl/GLVertexArray.h"

struct Material;
struct UpdateContext;

namespace mesh {
    class ModelMesh;
    class TexturedVAO;
    class SkinnedVAO;
}

class ModelRegistry {
public:
    static ModelRegistry& get() noexcept;

    ModelRegistry();
    ModelRegistry& operator=(const ModelRegistry&) = delete;

    ~ModelRegistry();

    void prepare(std::shared_ptr<std::atomic<bool>> alive);

    void updateRT(const UpdateContext& ctx);

    mesh::TexturedVAO* getTexturedVao()
    {
        return m_texturedVao.get();
    }

    mesh::SkinnedVAO* getSkinnedVao()
    {
        return m_skinnedVao.get();
    }

    std::shared_future<mesh::ModelMesh*> getMesh(
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

    std::unique_ptr<mesh::TexturedVAO> m_texturedVao;
    std::unique_ptr<mesh::SkinnedVAO> m_skinnedVao;
};
