#pragma once

#include <vector>
#include <map>
#include <mutex>

#include "asset/Assets.h"
#include "MeshType.h"

class MeshTypeRegistry {
public:
    MeshTypeRegistry(
        const Assets& assets,
        std::shared_ptr<std::atomic<bool>> alive);

    ~MeshTypeRegistry();

    MeshType* getType(
        const std::string& name);

private:
    const Assets& m_assets;

    std::shared_ptr<std::atomic<bool>> m_alive;

    std::mutex m_lock;
    std::vector<MeshType*> m_types;
};
