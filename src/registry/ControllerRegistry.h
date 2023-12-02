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

        for (auto* controller : it->second) {
            T* ptr = dynamic_cast<T*>(controller);
            if (ptr) return ptr;
        }
        return nullptr;
    }

    inline bool hasController(Node* node) const noexcept
    {
        if (!node) return false;

        const auto& it = m_controllers.find(node->m_id);
        return it != m_controllers.end() && !it->second.empty();
    }

    inline NodeController* getFirst(Node* node) const noexcept
    {
        if (!node) return nullptr;

        const auto& it = m_controllers.find(node->m_id);
        if (it == m_controllers.end()) return nullptr;
        return it->second[0];
    }

	inline const std::vector<NodeController*>* getControllers(Node* node) const noexcept
	{
        if (!node) return nullptr;

        const auto& it = m_controllers.find(node->m_id);
		if (it == m_controllers.end()) return nullptr;
		return &it->second;
	}

    void addController(
        ki::object_id targetId,
        NodeController* controller);

private:
    const Assets m_assets;

    Registry* m_registry{ nullptr };

    std::unordered_map<ki::object_id, std::vector<NodeController*>> m_controllers;
};
