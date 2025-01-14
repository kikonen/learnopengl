#pragma once

#include <string>

#include "asset/ViewportEffect.h"

struct LayerInfo
{
    static const int LAYER_NONE = 0;
    static const int LAYER_MAIN = 1;
    static const int LAYER_PLAYER = 2;
    static const int LAYER_UI = 3;

    std::string m_name;
    int m_index{ 0 };
    int m_order{ 0 };
    bool m_effectEnabled{ true };
    ViewportEffect m_effect{ ViewportEffect::none };
    bool m_blendEnabled{ false };
    float m_blendFactor{ 1.f };
};
