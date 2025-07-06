#include "LightDefinition.h"

#include "model/NodeType.h"

#include "component/definition/LightDefinition.h"
#include "component/Light.h"

std::unique_ptr<Light> LightDefinition::createLight(
    const NodeType* type)
{
    if (!type->m_lightDefinition) return nullptr;

    const auto& data = *type->m_lightDefinition;

    auto light = std::make_unique<Light>();

    light->m_enabled = true;

    light->setTargetId(data.m_targetId);

    light->m_linear = data.m_linear;
    light->m_quadratic = data.m_quadratic;

    light->m_cutoffAngle = data.m_cutoffAngle;
    light->m_outerCutoffAngle = data.m_outerCutoffAngle;

    light->m_diffuse = data.m_diffuse;
    light->m_intensity = data.m_intensity;

    light->m_type = data.m_type;

    return light;
}

