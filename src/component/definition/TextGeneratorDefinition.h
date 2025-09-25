#pragma once

#include <string>
#include <memory>

#include <glm/glm.hpp>

#include "text/Align.h"
#include "text/size.h"


namespace model
{
    class NodeType;
}

struct Material;

class TextGenerator;

struct TextGeneratorDefinition {
    std::string m_text;

    glm::vec2 m_pivot{ 0.f };
    text::Align m_alignHorizontal{ text::Align::left };
    text::Align m_alignVertical{ text::Align::top };

    std::shared_ptr<Material> m_material;

    text::font_id m_fontId;

    static std::unique_ptr<TextGenerator> createTextGenerator(
        const model::NodeType* type);
};
