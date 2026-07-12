#include "TextGeneratorDefinition.h"

#include "asset/Assets.h"

#include "model/NodeType.h"

#include "mesh/TextMesh.h"
#include "mesh/LodMeshContainer.h"

#include "material/Material.h"

#include "generator/TextGenerator.h"

//void TextGeneratorDefinition::setMaterial(const util::Ref<Material>& src) noexcept
//{
//    if (!src) {
//        m_material.reset();
//        return;
//    }
//
//    if (!m_material) {
//        m_material = util::Ref<Material>::create();
//    }
//    *m_material = *src;
//}

util::Ref<TextGenerator> TextGeneratorDefinition::createTextGenerator(
    const model::NodeType* type)
{
    if (!type->m_textGeneratorDefinition) return nullptr;

    const auto& data = *type->m_textGeneratorDefinition;

    const auto& assets = Assets::get();

    auto generator = util::Ref<TextGenerator>::create();

    generator->setText(data.m_text);
    generator->setMaxSize(data.m_maxSize);

    generator->setFontId(data.m_fontId);

    generator->setPivot(data.m_pivot);
    generator->setAlignHorizontal(data.m_alignHorizontal);
    generator->setAlignVertical(data.m_alignVertical);

    //generator->setMaterial(data.m_material);

    return generator;
}
