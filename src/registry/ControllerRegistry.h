#pragma once

#include <unordered_map>

#include "model/Node.h"

struct UpdateContext;
class Registry;

class NodeController;

class ControllerRegistry {
public:
    static ControllerRegistry& get() noexcept;

    ControllerRegistry()
    {
    }
    ControllerRegistry& operator=(const ControllerRegistry&) = delete;

    ~ControllerRegistry();

    void prepare(Registry* registry);

    void updateWT(const UpdateContext& ctx);

    template<typename T>
    inline T* get(Node* node) const noexcept
    {
        if (!node) return nullptr;
        if (!node->m_preparedRT) return nullptr;

        const auto& it = m_controllers.find(node->getId());
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
        if (!node->m_preparedRT) return false;

        const auto& it = m_controllers.find(node->getId());
        return it != m_controllers.end() && !it->second.empty();
    }

    inline NodeController* getFirst(Node* node) const noexcept
    {
        if (!node) return nullptr;
        if (!node->m_preparedRT) return nullptr;

        const auto& it = m_controllers.find(node->getId());
        return it != m_controllers.end() ? it->second[0] : nullptr;
    }

	inline const std::vector<NodeController*>* getControllers(Node* node) const noexcept
	{
        if (!node) return nullptr;
        if (!node->m_preparedRT) return nullptr;

        const auto& it = m_controllers.find(node->getId());
		return it != m_controllers.end() ? &it->second : nullptr;
	}

    void addController(
        ki::node_id targetId,
        NodeController* controller);

private:
    Registry* m_registry{ nullptr };

    std::unordered_map<ki::node_id, std::vector<NodeController*>> m_controllers;
};
