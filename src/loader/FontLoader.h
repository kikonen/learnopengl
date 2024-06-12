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
            const loader::DocNode& node,
            std::vector<FontData>& fonts) const;

        void loadFont(
            const loader::DocNode& node,
            FontData& data) const;

        void createFonts(
            std::vector<FontData>& data);

        text::font_id createFont(
            const FontData& data) const;

        text::font_id resolveFont(
            pool::TypeHandle typeHandle,
            const TextData& data) const;
    };
}
