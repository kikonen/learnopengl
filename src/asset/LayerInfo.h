#pragma once

#include <string>

#include "asset/ViewportEffect.h"

constexpr int LAYER_NONE_INDEX{ 0 };

const std::string LAYER_MAIN{ "main" };
const std::string LAYER_REAR{ "rear" };
const std::string LAYER_PLAYER{ "player" };
const std::string LAYER_UI{ "ui" };

struct LayerInfo
{
    std::string m_name;
    int m_index{ 0 };
    int m_order{ 0 };
    bool m_enabled{ true };
    bool m_effectEnabled{ true };
    ViewportEffect m_effect{ ViewportEffect::none };
    bool m_blendEnabled{ false };
    float m_blendFactor{ 1.f };

    static const LayerInfo* findLayer(const std::string& name);
};
