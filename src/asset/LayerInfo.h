#pragma once

#include <string>

#include "asset/ViewportEffect.h"

struct LayerInfo
{
    std::string m_name;
    int m_index{ 0 };
    int m_order{ 0 };
    bool m_effectEnabled{ true };
    ViewportEffect m_effect{ ViewportEffect::none };
    bool m_blendEnabled{ false };
    float m_blendFactor{ 1.f };
};
