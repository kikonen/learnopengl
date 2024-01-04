#pragma once

#include <vector>

#include "text/size.h"

#include "BaseLoader.h"
#include "FontData.h"

namespace loader {
    class FontLoader : public BaseLoader
    {
    public:
        FontLoader(
            Context ctx);

        void loadFonts(
            const YAML::Node& node,
            std::vector<FontData>& fonts) const;

        void loadFont(
            const YAML::Node& node,
            FontData& data) const;

        void createFonts(
            std::vector<FontData>& data);

        text::font_id createFont(
            const FontData& data) const;
    };
}
