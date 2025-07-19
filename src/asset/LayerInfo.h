#pragma once

#include <string>
#include <stdint.h>

#include "asset/ViewportEffect.h"

inline constexpr uint8_t LAYER_NONE_INDEX{ 0 };

inline const std::string LAYER_MAIN{ "main" };
inline const std::string LAYER_REAR{ "rear" };
inline const std::string LAYER_PLAYER{ "player" };
inline const std::string LAYER_UI{ "ui" };

struct LayerInfo
{
    std::string m_name;
    uint8_t m_index{ 0 };
    int m_order{ 0 };
    bool m_enabled{ true };
    bool m_effectEnabled{ true };
    ViewportEffect m_effect{ ViewportEffect::none };
    bool m_blendEnabled{ false };
    float m_blendFactor{ 1.f };

    static const LayerInfo* findLayer(const std::string& name);
};
