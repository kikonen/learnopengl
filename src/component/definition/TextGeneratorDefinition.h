#pragma once

#include <string>
#include <memory>

#include <glm/glm.hpp>

#include "util/Ref.h"

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
    uint32_t m_maxSize{ 100 };

    glm::vec2 m_pivot{ 0.f };
    text::Align m_alignHorizontal{ text::Align::left };
    text::Align m_alignVertical{ text::Align::top };

    //util::Ref<Material> m_material;

    text::font_id m_fontId;

    //void setMaterial(const util::Ref<Material>& src) noexcept;

    static util::Ref<TextGenerator> createTextGenerator(
        const model::NodeType* type);
};
