#include "TextLoader.h"

#include "asset/Assets.h"

#include "mesh/MeshFlags.h"

#include "loader/document.h"
#include "Loaders.h"

#include "generator/TextGenerator.h"
#include "loader_util.h"

namespace {
    std::map<std::string, text::Align> g_alignments{
    { "left", text::Align::left},
    { "right", text::Align::right},
    { "center", text::Align::center},
    { "top", text::Align::top},
    { "bottom", text::Align::bottom},
    };

    text::Align readAlign(const loader::DocNode& node) {
        const auto& v = readString(node);
        const auto& it = g_alignments.find(v);
        if (it != g_alignments.end()) return it->second;
        KI_WARN_OUT(fmt::format("LOADER_TEXT: missing_alignment={}", v));
        return text::Align::none;
    }
}

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
            else if (k == "pivot") {
                data.pivot = readVec2(v);
            }
            else if (k == "align_horizontal") {
                data.alignHorizontal = readAlign(v);
            }
            else if (k == "align_vertical") {
                data.alignVertical = readAlign(v);
            }
            else if (k == "material") {
                loaders.m_materialLoader.loadMaterial(v, data.materialData, loaders);
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
        generator->setPivot(data.pivot);
        generator->setAlignHorizontal(data.alignHorizontal);
        generator->setAlignVertical(data.alignVertical);

        generator->m_material = data.materialData.material;
        generator->m_material.loadTextures();

        loaders.m_materialLoader.resolveProgram({}, generator->m_material);

        return generator;
    }
}
