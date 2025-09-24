#pragma once

#include <unordered_map>
#include <memory>

#include "model/Node.h"

struct UpdateContext;
class Registry;

class NodeController;

class ControllerRegistry {
public:
    static void init() noexcept;
    static void release() noexcept;
    static ControllerRegistry& get() noexcept;

    ControllerRegistry();
    ~ControllerRegistry();

    ControllerRegistry& operator=(const ControllerRegistry&) = delete;

    void clear();

    void shutdown();
    void prepare(Registry* registry);

    void updateWT(const UpdateContext& ctx);

    template<typename T>
    inline T* get(model::Node* node) const noexcept
    {
        if (!node) return nullptr;
        if (!node->m_preparedRT) return nullptr;

        const auto& it = m_controllers.find(node->toHandle());
        if (it == m_controllers.end()) return nullptr;

        for (auto* controller : it->second) {
            T* ptr = dynamic_cast<T*>(controller);
            if (ptr) return ptr;
        }
        return nullptr;
    }

    inline bool hasController(model::Node* node) const noexcept
    {
        if (!node) return false;
        if (!node->m_preparedRT) return false;

        const auto& it = m_controllers.find(node->toHandle());
        return it != m_controllers.end() && !it->second.empty();
    }

    inline NodeController* getFirst(model::Node* node) const noexcept
    {
        if (!node) return nullptr;
        if (!node->m_preparedRT) return nullptr;

        const auto& it = m_controllers.find(node->toHandle());
        return it != m_controllers.end() ? it->second[0].get() : nullptr;
    }

	inline const std::vector<std::unique_ptr<NodeController>>* forNode(model::Node* node) const noexcept
	{
        if (!node) return nullptr;
        if (!node->m_preparedRT) return nullptr;

        const auto& it = m_controllers.find(node->toHandle());
		return it != m_controllers.end() ? &it->second : nullptr;
	}

    const std::unordered_map<pool::NodeHandle, std::vector<std::unique_ptr<NodeController>>>& getControllers() const noexcept
    {
        return m_controllers;
    }

    void addController(
        pool::NodeHandle target,
        std::unique_ptr<NodeController> controller);

private:
    Registry* m_registry{ nullptr };

    std::unordered_map<pool::NodeHandle, std::vector<std::unique_ptr<NodeController>>> m_controllers;
};
