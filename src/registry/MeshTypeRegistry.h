#pragma once

#include <vector>
#include <map>
#include <mutex>

#include "asset/Assets.h"
#include "MeshType.h"

class MeshTypeRegistry {
public:
    MeshTypeRegistry(const Assets& assets);
    ~MeshTypeRegistry();

    MeshType* getType(
        const std::string& name);

private:
    const Assets& assets;

    std::mutex m_lock;
    std::vector<MeshType*> m_types;
};
