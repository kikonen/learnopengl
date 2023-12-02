#pragma once

#include <unordered_map>

#include "asset/Assets.h"

#include "model/Node.h"

class Registry;
class NodeController;
class UpdateContext;

class ControllerRegistry {
public:
    ControllerRegistry(const Assets& assets)
        : m_assets(assets)
    {
    }

    ~ControllerRegistry();

    void prepare(Registry* registry);

    void update(const UpdateContext& ctx);

    template<typename T>
    inline T* get(Node* node) const noexcept
    {
        const auto& it = m_controllers.find(node->m_id);
        if (it == m_controllers.end()) return nullptr;
        return dynamic_cast<T*>(it->second);
    }

    void addController(
        ki::object_id targetId,
        NodeController* controller);

private:
    const Assets m_assets;

    Registry* m_registry{ nullptr };

    std::unordered_map<ki::object_id, NodeController*> m_controllers;
};
