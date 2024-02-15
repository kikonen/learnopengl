#include "FontLoader.h"

#include "ki/yaml.h"
#include "util/Util.h"

#include "text/FontAtlas.h"

#include "registry/Registry.h"
#include "registry/FontRegistry.h"


namespace loader {
    FontLoader::FontLoader(
        Context ctx)
        : BaseLoader(ctx)
    {
    }

    void FontLoader::loadFonts(
        const YAML::Node& node,
        std::vector<FontData>& fonts) const
    {
        for (const auto& entry : node) {
            FontData& data = fonts.emplace_back();
            loadFont(entry, data);
        }
    }

    void FontLoader::loadFont(
        const YAML::Node& node,
        FontData& data) const
    {
        for (const auto& pair : node) {
            const std::string& k = pair.first.as<std::string>();
            const YAML::Node& v = pair.second;

            if (k == "name") {
                data.name = readString(v);
            }
            else if (k == "path") {
                data.path = readString(v);
            }
            else if (k == "size") {
                data.size = readFloat(v);
            }
            else if (k == "atlas_size") {
                data.atlasSize = readVec2(v);
            }
            else {
                reportUnknown("font_entry", k, v);
            }
        }
    }

    void FontLoader::createFonts(
        std::vector<FontData>& fonts)
    {
        for (auto& data : fonts) {
            data.id = createFont(data);
        }
    }

    text::font_id FontLoader::createFont(
        const FontData& data) const
    {
        auto& fr = FontRegistry::get();
        auto id = fr.registerFont(data.name);

        auto* font = fr.modifyFont(id);
        font->m_fontPath = data.path;
        font->m_fontSize = data.size;
        font->m_atlasSize = data.atlasSize;

        // TODO KI race condition with RT
        //font->prepare();

        return id;
    }
}
