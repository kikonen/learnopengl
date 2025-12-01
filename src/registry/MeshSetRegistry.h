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

class MeshSetRegistry {
public:
    static void init() noexcept;
    static void release() noexcept;
    static MeshSetRegistry& get() noexcept;

    MeshSetRegistry();
    MeshSetRegistry& operator=(const MeshSetRegistry&) = delete;

    ~MeshSetRegistry();

    void clear();

    void prepare(std::shared_ptr<std::atomic_bool> alive);

    std::shared_future<std::shared_ptr<mesh::MeshSet>> getMeshSet(
        std::string_view id,
        std::string_view rootDir,
        std::string_view meshPath,
        bool smoothNormals,
        bool forceNormals);

private:
    std::shared_future<std::shared_ptr<mesh::MeshSet>> startLoad(
        std::shared_ptr<mesh::MeshSet> meshSet);

private:
    std::shared_ptr<std::atomic_bool> m_alive;

    std::mutex m_meshes_lock{};
    std::unordered_map<const std::string, std::shared_future<std::shared_ptr<mesh::MeshSet>>, util::constant_string_hash> m_meshes;

    std::unique_ptr<Material> m_defaultMaterial{ nullptr };
    bool m_forceDefaultMaterial = false;
};
