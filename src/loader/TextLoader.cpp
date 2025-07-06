#include "TextLoader.h"

#include "asset/Assets.h"

#include "mesh/MeshFlags.h"

#include "component/definition/TextGeneratorDefinition.h"

#include "loader/document.h"
#include "Loaders.h"

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
        std::shared_ptr<Context> ctx)
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

    std::unique_ptr<TextGeneratorDefinition> TextLoader::createDefinition(
        const NodeType* type,
        const TextData& data,
        Loaders& loaders)
    {
        if (!data.enabled) return nullptr;

        const auto& assets = Assets::get();

        auto definition = std::make_unique<TextGeneratorDefinition>();
        auto& df = *definition;

        df.m_text = data.text;
        df.m_pivot = data.pivot;

        df.m_alignHorizontal = data.alignHorizontal;
        df.m_alignVertical = data.alignVertical;

        df.m_material = std::make_shared<Material>();

        auto& material = *df.m_material;
        material = data.materialData.material;
        material.loadTextures();
        loaders.m_materialLoader.resolveProgram({}, material);

        df.m_fontId = loaders.m_fontLoader.resolveFont(data.fontData);

        return definition;
    }
}
