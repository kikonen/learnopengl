#include "TextGeneratorDefinition.h"

#include "asset/Assets.h"

#include "model/NodeType.h"

#include "mesh/TextMesh.h"
#include "mesh/LodMeshContainer.h"

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

    {
        // NOTE KI store direct mesh refesh to generator to avoid redundant querying
        auto* lodMesh = type->getMeshContainer()->getLodMesh(0);
        mesh::TextMesh* mesh = lodMesh->getMesh<mesh::TextMesh>();

        generator->setMesh(mesh);
    }
    return generator;
}
