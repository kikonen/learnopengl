#include "TextGeneratorDefinition.h"

#include "asset/Assets.h"

#include "model/NodeType.h"

#include "generator/TextGenerator.h"

std::unique_ptr<TextGenerator> TextGeneratorDefinition::createTextGenerator(
    const model::NodeType* type)
{
    if (!type->m_textGeneratorDefinition) return nullptr;

    const auto& data = *type->m_textGeneratorDefinition;

    const auto& assets = Assets::get();

    auto generator = std::make_unique<TextGenerator>();

    generator->setFontId(data.m_fontId);
    generator->setText(data.m_text);
    generator->setPivot(data.m_pivot);
    generator->setAlignHorizontal(data.m_alignHorizontal);
    generator->setAlignVertical(data.m_alignVertical);

    generator->m_material = *data.m_material;

    return generator;
}

