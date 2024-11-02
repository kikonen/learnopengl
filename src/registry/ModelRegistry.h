#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <mutex>
#include <future>

#include "glm/glm.hpp"

#include "util/util.h"

struct Material;

namespace mesh {
    class MeshSet;
}

class ModelRegistry {
public:
    static ModelRegistry& get() noexcept;

    ModelRegistry();
    ModelRegistry& operator=(const ModelRegistry&) = delete;

    ~ModelRegistry();

    void prepare(std::shared_ptr<std::atomic<bool>> alive);

    std::shared_future<mesh::MeshSet*> getMeshSet(
        std::string_view id,
        std::string_view rootDir,
        std::string_view meshPath);

private:
    std::shared_future<mesh::MeshSet*> startLoad(mesh::MeshSet* mesh);

private:
    std::shared_ptr<std::atomic<bool>> m_alive;

    std::mutex m_meshes_lock{};
    std::unordered_map<const std::string, std::shared_future<mesh::MeshSet*>, util::constant_string_hash> m_meshes;

    std::unique_ptr<Material> m_defaultMaterial{ nullptr };
    bool m_forceDefaultMaterial = false;
};
