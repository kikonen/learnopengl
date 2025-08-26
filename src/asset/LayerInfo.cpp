#include "LayerInfo.h"

#include "debug/DebugContext.h"

const LayerInfo* LayerInfo::findLayer(const std::string& name)
{
    const auto& dbg = debug::DebugContext::get();

    for (const auto& layer : dbg.m_layers) {
        if (layer.m_name == name) return &layer;
    }

    return nullptr;
}
