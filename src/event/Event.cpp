#include "Event.h"

#include "scene/UpdateContext.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"

namespace event {
    void NodeAdd::dispatch(const UpdateContext& ctx) {
        auto* nodeRegistry = ctx.m_registry->m_nodeRegistry.get();
        nodeRegistry->addNode(m_node);
    }
}
