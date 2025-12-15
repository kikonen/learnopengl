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
            const std::shared_ptr<Context>& ctx);

        void loadFonts(
            const loader::DocNode& node,
            std::vector<FontData>& fonts) const;

        void loadFont(
            const loader::DocNode& node,
            FontData& data) const;

        text::font_id resolveFont(
            const FontData& data) const;
    };
}
