#include "TextLoader.h"

#include "asset/Assets.h"

#include "loader/document.h"
#include "Loaders.h"

#include "generator/TextGenerator.h"


namespace loader {
    TextLoader::TextLoader(
        Context ctx)
        : BaseLoader(ctx)
    {
    }

    void TextLoader::loadText(
        const loader::DocNode& node,
        TextData& data,
        Loaders& loaders) const
    {
        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "xtext") {
                data.text = readString(v);
                data.enabled = false;
            }
            else if (k == "text") {
                data.text = readString(v);
                data.enabled = true;
            }
            else if (k == "font") {
                loaders.m_fontLoader.loadFont(v, data.fontData);
            }
            else if (k == "material") {
                loaders.m_materialLoader.loadMaterial(v, data.materialData);
            }
            else {
                reportUnknown("text_entry", k, v);
            }
        }
    }

    std::unique_ptr<NodeGenerator> TextLoader::createGenerator(
        const mesh::MeshType* type,
        const TextData& data,
        Loaders& loaders)
    {
        if (!data.enabled) return nullptr;

        const auto& assets = Assets::get();

        auto generator = std::make_unique<TextGenerator>();

        auto fontId = loaders.m_fontLoader.resolveFont(data.fontData);
        generator->setFontId(fontId);
        generator->setText(data.text);

        generator->m_material = data.materialData.material;
        generator->m_material.loadTextures();
        loaders.m_materialLoader.resolveProgram(type, generator->m_material);

        return generator;
    }
}
