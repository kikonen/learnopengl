#include "FontLoader.h"

#include "util/util.h"

#include "text/FontAtlas.h"
#include "text/FontRegistry.h"

#include "loader/document.h"
#include "loader_util.h"

namespace loader {
    FontLoader::FontLoader(
        Context ctx)
        : BaseLoader(ctx)
    {
    }

    void FontLoader::loadFonts(
        const loader::DocNode& node,
        std::vector<FontData>& fonts) const
    {
        for (const auto& entry : node.getNodes()) {
            FontData& data = fonts.emplace_back();
            loadFont(entry, data);
        }
    }

    void FontLoader::loadFont(
        const loader::DocNode& node,
        FontData& data) const
    {
        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "name") {
                data.name = readString(v);
            }
            else if (k == "path") {
                data.path = readString(v);
            }
            else if (k == "size") {
                data.size = readFloat(v);
            }
            else {
                reportUnknown("font_entry", k, v);
            }
        }
    }

    text::font_id FontLoader::resolveFont(
        const FontData& data) const
    {
        auto& fr = text::FontRegistry::get();

        text::FontAtlas font;
        font.m_name = data.name;
        font.m_fontPath = data.path;
        font.m_fontSize = data.size;

        return fr.registerFont(std::move(font));
    }
}
