#pragma once

#include <string>
#include <memory>

#include <glm/glm.hpp>

#include "text/Align.h"
#include "text/size.h"

struct Material;

struct TextDefinition {
    std::string m_text;

    glm::vec2 m_pivot{ 0.f };
    text::Align m_alignHorizontal{ text::Align::left };
    text::Align m_alignVertical{ text::Align::top };

    std::shared_ptr<Material> m_material;

    text::font_id m_fontId;
};
